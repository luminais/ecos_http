#include <config.h>
#include <includes.h>
#include <radvd.h>
#include <pathnames.h>

const char * RADVD_CONF_SAMPLE = "\
interface eth0\n\
{\n\
	AdvSendAdvert on;\n\
\n\
# This may be needed on some interfaces which are not active when\n\
# radvd starts, but becomoe available later on, and are activated by sending a\n\
# HUP signal to radvd; see man page for details.\n\
\n\
	# IgnoreIfMissing on;\n\
\n\
#\n\
# These settings cause advertisements to be sent every 3-10 seconds.  This\n\
# range is good for 6to4 with a dynamic IPv4 address, but can be greatly\n\
# increased when not using 6to4 prefixes.\n\
#\n\
\n\
	MinRtrAdvInterval 3;\n\
	MaxRtrAdvInterval 10;\n\
\n\
#\n\
# You can use AdvDefaultPreference setting to advertise the preference of\n\
# the router for the purposes of default router determination.\n\
# NOTE: This feature is still being specified and is not widely supported!\n\
#\n\
	AdvDefaultPreference low;\n\
\n\
#\n\
# Disable Mobile IPv6 support\n\
#\n\
	AdvHomeAgentFlag off;\n\
\n\
#\n\
# example of a standard prefix\n\
#\n\
	prefix 2001:db8:1:0::/64\n\
	{\n\
		AdvOnLink on;\n\
		AdvAutonomous on;\n\
		AdvRouterAddr off;\n\
	};\n\
\n\
#\n\
# example of a more specific route\n\
# NOTE: This feature is still being specified and is not widely supported!\n\
#\n\
#	route 2001:db0:fff::/48\n\
#	{\n\
#		AdvRoutePreference high;\n\
#		AdvRouteLifetime 3600;\n\
#	};\n\
\n\
#\n\
# example of a 6to4 prefix\n\
#\n\
# Note that the first 48 bits are specified here as zeros.  These will be\n\
# replaced with the appropriate 6to4 address when radvd starts or is\n\
# reconfigured. Be sure that the SLA ID (1234 in this case) is specified\n\
# here!\n\
#\n\
	prefix 0:0:0:1234::/64\n\
	{\n\
		AdvOnLink on;\n\
		AdvAutonomous on;\n\
		AdvRouterAddr off;\n\
\n\
#\n\
# This setting causes radvd to replace the first 48 bits of the prefix\n\
# with the 6to4 address generated from the specified interface.  For example,\n\
# if the address of ppp0 is 192.0.2.25 when radvd configures itself, this\n\
# prefix will be advertised as 2002:C000:0219:1234::/64.\n\
#\n\
# If ppp0 is not available at configuration time, this prefix will not be\n\
# advertised, but other prefixes listed in the configuration will be\n\
# advertised as usual.\n\
#\n\
# When using the Base6to4Interface option, make sure radvd receives a\n\
# SIGHUP every time the ppp0 interface goes up, down, or is assigned a\n\
# new IPv4 address.  The SIGHUP will cause radvd to recognize that the\n\
# ppp0 interface has changed and will adjust the advertisements\n\
# accordingly.\n\
#\n\
\n\
		Base6to4Interface eth0;\n\
\n\
#\n\
# If the IP address of ppp0 is assigned dynamically, be sure to set the\n\
# lifetimes for this prefix to be small.  Otherwise, hosts on your network\n\
# may continue to use a prefix that no longer corresponds to the address\n\
# on ppp0!\n\
#\n\
		AdvPreferredLifetime 120;\n\
		AdvValidLifetime 300;\n\
	};\n\
\n\
};\n\
";

// MIB structure 
#define IFNAMESIZE		32
#define MAX_PREFIX_NUM	2

typedef struct {
	bool enable;
	unsigned short addr[ 8 ];
	int  prefix_length;
	bool AdvOnLink;
	bool AdvAutonomous;
	bool AdvRouterAddr;
	uint32_t  AdvValidLifetime;		// sec 
	uint32_t  AdvPreferredLifetime;	// sec 
	char Base6to4Interface[ IFNAMESIZE ];
	//int  AdvRouteLifetime;	// sec 
	//int  AdvRoutePreference;	// low | medium | high
} prefix_t;

typedef struct {
	bool enable;
	char interface[ IFNAMESIZE ];
	bool IgnoreIfMissing;
	bool AdvSendAdvert;
	bool UnicastOnly;
	int  MaxRtrAdvInterval;		// sec
	int  MinRtrAdvInterval;		// sec
	int  MinDelayBetweenRAs;	// sec
	bool AdvManagedFlag;
	bool AdvOtherConfigFlag;
	int  AdvLinkMTU;
	uint32_t  AdvReachableTime;		// msec 
	uint32_t  AdvRetransTimer;		// msec 
	int  AdvCurHopLimit;
	int  AdvDefaultLifetime;	// sec 
	int  AdvDefaultPreference;	// low | medium | high
	bool AdvSourceLLAddress;
	//bool AdvHomeAgentFlag;
	//bool AdvHomeAgentInfo;
	//int  HomeAgentLifetime;	// sec
	//int  HomeAgentPreference;
	//bool AdvMobRtrSupportFlag;
	//bool AdvIntervalOpt;
	
	prefix_t prefix[ MAX_PREFIX_NUM ];
} interface_t;

typedef struct {
	interface_t interface;
} conf_t;

static conf_t conf;	// note: conf is *NOT* volatile, so we don't store! 
static const conf_t conf_default = {
	{
		.enable = 1,
		.interface = "eth0",
		.IgnoreIfMissing = 0,
		.AdvSendAdvert = 1,
		.UnicastOnly = 0,
		.MaxRtrAdvInterval = 15,//600,			// sec
		.MinRtrAdvInterval = 5,//0.33 * 600,	// sec
		.MinDelayBetweenRAs = 3,			// sec
		.AdvManagedFlag = 0,
		.AdvOtherConfigFlag = 0,
		.AdvLinkMTU = 0,
		.AdvReachableTime = 0,			// msec 
		.AdvRetransTimer = 0,			// msec 
		.AdvCurHopLimit = 64,
		.AdvDefaultLifetime =  3 * 600,	// sec 
		.AdvDefaultPreference = 1,		// low | medium | high
		.AdvSourceLLAddress = 1,
		.prefix[ 0 ] = {
			.enable = 1,
			.addr = { 0x2001, 0xdb8, 0x1, 0x0 },
			.prefix_length = 64, 
			.AdvOnLink = 1,
			.AdvAutonomous = 1,
			.AdvRouterAddr = 0,
			.AdvValidLifetime = 2592000,		// sec 
			.AdvPreferredLifetime = 604800,		// sec 
			.Base6to4Interface = "",
			//.AdvRouteLifetime = 3 * 600,		// sec 
			//.AdvRoutePreference = 1			// low | medium | high
		},
		.prefix[ 1 ] = {
			.enable = 1,
			.addr = { 0x0, 0x0, 0x0, 0x1234 },
			.prefix_length = 64, 
			.AdvOnLink = 1,
			.AdvAutonomous = 1,
			.AdvRouterAddr = 0,
			.AdvValidLifetime = 300,			// sec 
			.AdvPreferredLifetime = 120,		// sec 
			.Base6to4Interface = "eth0",
			//.AdvRouteLifetime = 3 * 600,		// sec 
			//.AdvRoutePreference = 1			// low | medium | high
		},
	}
};

// text <-> binary descriptor  
#define FIELD_OFFSET( t, f )		( ( int )&( ( ( t* )0 ) ->f ) )
#define FIELD_SIZE( t, f )			( sizeof( ( ( t* )0 ) ->f ) )

#define M_BASE( f )			FIELD_OFFSET( conf_t, f ), FIELD_SIZE( conf_t, f ), \
								#f, sizeof( #f ) - 1

#define M_INT( f )			{ M_BASE( f ), OT_INT , }
#define M_BOOL( f )			{ M_BASE( f ), OT_BOOL, }
#define M_CHAR( f )			{ M_BASE( f ), OT_CHAR, }
#define M_IN6A( f )			{ M_BASE( f ), OT_IN6A, }

typedef enum {
	OT_INT,
	OT_BOOL,
	OT_CHAR,
	OT_IN6A, 
} ot_t;

typedef struct option_s {
	int offset;
	int size;
	const char *name;
	int name_len;
	ot_t type;
} option_t;

static const option_t options[] = {
	
	M_BOOL( interface.enable ),
	M_CHAR( interface.interface ),			// IFNAMESIZE
	M_BOOL( interface.IgnoreIfMissing ),
	M_BOOL( interface.AdvSendAdvert ),
	M_BOOL( interface.UnicastOnly ),
	M_INT ( interface.MaxRtrAdvInterval ),		// sec
	M_INT ( interface.MinRtrAdvInterval ),		// sec
	M_INT ( interface.MinDelayBetweenRAs ),	// sec
	M_BOOL( interface.AdvManagedFlag ),
	M_BOOL( interface.AdvOtherConfigFlag ),
	M_INT ( interface.AdvLinkMTU ),
	M_INT ( interface.AdvReachableTime ),		// msec 
	M_INT ( interface.AdvRetransTimer ),		// msec 
	M_INT ( interface.AdvCurHopLimit ),
	M_INT ( interface.AdvDefaultLifetime ),	// sec 
	M_INT ( interface.AdvDefaultPreference ),	// low | medium | high
	M_BOOL( interface.AdvSourceLLAddress ),

	M_BOOL( interface.prefix[0].enable ),
	M_IN6A( interface.prefix[0].addr ),
	M_INT ( interface.prefix[0].prefix_length ),
	M_BOOL( interface.prefix[0].AdvOnLink ),
	M_BOOL( interface.prefix[0].AdvAutonomous ),
	M_BOOL( interface.prefix[0].AdvRouterAddr ),
	M_INT ( interface.prefix[0].AdvValidLifetime ),		// sec 
	M_INT ( interface.prefix[0].AdvPreferredLifetime ),	// sec 
	M_CHAR( interface.prefix[0].Base6to4Interface ),	// IFNAMESIZE 
	//M_INT ( interface.prefix[0].AdvRouteLifetime ),		// sec 
	//M_INT ( interface.prefix[0].AdvRoutePreference ),	// low | medium | high

	M_BOOL( interface.prefix[1].enable ),
	M_IN6A( interface.prefix[1].addr ),
	M_INT ( interface.prefix[1].prefix_length ),
	M_BOOL( interface.prefix[1].AdvOnLink ),
	M_BOOL( interface.prefix[1].AdvAutonomous ),
	M_BOOL( interface.prefix[1].AdvRouterAddr ),
	M_INT ( interface.prefix[1].AdvValidLifetime ),		// sec 
	M_INT ( interface.prefix[1].AdvPreferredLifetime ),	// sec 
	M_CHAR( interface.prefix[1].Base6to4Interface ),	// IFNAMESIZE 
	//M_INT ( interface.prefix[1].AdvRouteLifetime ),		// sec 
	//M_INT ( interface.prefix[1].AdvRoutePreference ),	// low | medium | high

};

#define OPTIONS_SIZE		( sizeof( options ) / sizeof( options[ 0 ] ) )


// 
static void write_conf_sample( void )
{
	FILE *fp;
	
	fp = fopen( PATH_RADVD_CONF, "w" );
	
	fputs( RADVD_CONF_SAMPLE, fp );
	
	fclose( fp );
}

static void write_conf_options( void )
{
	FILE *fp;
	int i;
	interface_t *intf = &conf.interface;
	
	fp = fopen( PATH_RADVD_CONF, "w" );
	
	if( !intf ->enable ) 
		goto label_done;
	
	fprintf( fp, 
		"interface %s\n"
		"{\n"
		"	IgnoreIfMissing %s;\n"
		"	AdvSendAdvert %s;\n"
		"	UnicastOnly %s;\n"
		"	MaxRtrAdvInterval %d;\n"
		"	MinRtrAdvInterval %d;\n"
		"	MinDelayBetweenRAs %d;\n"
		"	AdvManagedFlag %s;\n"
		"	AdvOtherConfigFlag %s;\n"
		"	AdvLinkMTU %d;\n"
		"	AdvReachableTime %u;\n"
		"	AdvRetransTimer %u;\n"
		"	AdvCurHopLimit %d;\n"
		"	AdvDefaultLifetime %d;\n"
		"	AdvDefaultPreference %s;\n"
		"	AdvSourceLLAddress %s;\n"
		,
		intf ->interface, 
		( intf ->IgnoreIfMissing ? "on" : "off" ), 
		( intf ->AdvSendAdvert ? "on" : "off" ), 
		( intf ->UnicastOnly ? "on" : "off" ), 
		intf ->MaxRtrAdvInterval,
		intf ->MinRtrAdvInterval,
		intf ->MinDelayBetweenRAs,
		( intf ->AdvManagedFlag ? "on" : "off" ),
		( intf ->AdvOtherConfigFlag ? "on" : "off" ),
		intf ->AdvLinkMTU,
		intf ->AdvReachableTime,
		intf ->AdvRetransTimer,
		intf ->AdvCurHopLimit,
		intf ->AdvDefaultLifetime,
		( intf ->AdvDefaultPreference == 0 ? "low" :
			( intf ->AdvDefaultPreference == 2 ? "high" : "medium" ) ),
		( intf ->AdvSourceLLAddress ? "on" : "off" ) );
		
	for( i = 0; i < MAX_PREFIX_NUM; i ++ ) {
	
		const prefix_t *prefix = &intf ->prefix[ i ];
		
		if( !prefix ->enable )
			continue;
		fprintf( fp, 
			"	prefix %X:%X:%X:%X:%X:%X:%X:%X/%d\n"
			"	{\n"
			"		AdvOnLink %s;\n"
			"		AdvAutonomous %s;\n"
			"		AdvRouterAddr %s;\n"
			"		AdvValidLifetime %u;\n"
			"		AdvPreferredLifetime %u;\n"
			"		%s %s%s\n"
			"	};\n"
			,
			prefix ->addr[ 0 ], prefix ->addr[ 1 ], prefix ->addr[ 2 ], prefix ->addr[ 3 ], 
			prefix ->addr[ 4 ], prefix ->addr[ 5 ], prefix ->addr[ 6 ], prefix ->addr[ 7 ], 
			prefix ->prefix_length, 
			( prefix ->AdvOnLink ? "on" : "off" ),
			( prefix ->AdvAutonomous ? "on" : "off" ),
			( prefix ->AdvRouterAddr ? "on" : "off" ),
			prefix ->AdvValidLifetime,
			prefix ->AdvPreferredLifetime,
			( prefix ->Base6to4Interface[ 0 ] ? "Base6to4Interface" : "" ), 
			prefix ->Base6to4Interface,
			( prefix ->Base6to4Interface[ 0 ] ? ";" : "" )
		);
	}
	
	fprintf( fp, "};\n" );
	
label_done:	
	fclose( fp );
}

static void options_default( void )
{
	conf = conf_default;
}

static void options_dump( void )
{
#define OFFSET2ADDR2( off )				( ( ( int )( void * )&conf ) + ( off ) )
#define OFFSET2ADDR( off, type )		*( ( type * )( ( ( int )( void * )&conf ) + ( off ) ) )
	int i;
	const option_t *popt;
	
	for( i = 0; i < OPTIONS_SIZE; i ++ ) {
		
		popt = &options[ i ];
		
		switch( popt ->type ) {
		case OT_INT:
			printf( "%s = %d\n", popt ->name, OFFSET2ADDR( popt ->offset, int ) );
			break;
			
		case OT_BOOL:
			printf( "%s = %d\n", popt ->name, OFFSET2ADDR( popt ->offset, bool ) );
			break;
			
		case OT_CHAR:
			printf( "%s = %s\n", popt ->name, ( char * )OFFSET2ADDR2( popt ->offset ) );
			break;
			
		case OT_IN6A:
			printf( "%s = %04X:%04X:%04X:%04X:%04X:%04X:%04X:%04X\n", popt ->name, 
						OFFSET2ADDR( popt ->offset + 0, unsigned short ),
						OFFSET2ADDR( popt ->offset + 2, unsigned short ),
						OFFSET2ADDR( popt ->offset + 4, unsigned short ),
						OFFSET2ADDR( popt ->offset + 6, unsigned short ),
						OFFSET2ADDR( popt ->offset + 8, unsigned short ),
						OFFSET2ADDR( popt ->offset + 10, unsigned short ),
						OFFSET2ADDR( popt ->offset + 12, unsigned short ),
						OFFSET2ADDR( popt ->offset + 14, unsigned short )
						 );
			break;
		}
	}
#undef OFFSET2ADDR
#undef OFFSET2ADDR2
}

static int parse_option( const char *str )
{
#define OFFSET2ADDR2( off )				( ( ( int )( void * )&conf ) + ( off ) )

	int i;
	const option_t *popt;
	const char *pval;
	int ret = 0;
	
	if( str[ 0 ] == '-' )
		return;
	
	for( i = 0; i < OPTIONS_SIZE; i ++ ) {
		
		popt = &options[ i ];
		
		if( memcmp( popt ->name, str, popt ->name_len ) == 0 &&
			str[ popt ->name_len ] == '=' )
		{
			// match 'xxxx=' 
			pval = &str[ popt ->name_len + 1 ];
		} else
			continue;
		
		switch( popt ->type ) {
		case OT_INT:
		case OT_BOOL:
			ret = sscanf( pval, "%u", OFFSET2ADDR2( popt ->offset ) );
			break;
			
		case OT_CHAR:
			memcpy( OFFSET2ADDR2( popt ->offset ), pval, popt ->size );
			*( ( ( char * )OFFSET2ADDR2( popt ->offset ) ) + popt ->size ) = '\x0';
			ret = 1;
			break;
			
		case OT_IN6A:
			ret = sscanf( pval, "%hX:%hX:%hX:%hX:%hX:%hX:%hX:%hX", 
						OFFSET2ADDR2( popt ->offset ) + 0, 
						OFFSET2ADDR2( popt ->offset ) + 2, 
						OFFSET2ADDR2( popt ->offset ) + 4, 
						OFFSET2ADDR2( popt ->offset ) + 6, 
						OFFSET2ADDR2( popt ->offset ) + 8, 
						OFFSET2ADDR2( popt ->offset ) + 10, 
						OFFSET2ADDR2( popt ->offset ) + 12, 
						OFFSET2ADDR2( popt ->offset ) + 14 );
			break; 
		}
		
		return ret;
	}
#undef OFFSET2ADDR2

	return 0;
}

static void usage( void )
{
	printf( "Options:\n"
			"	-apply-sample\n"
			"	-as\n"
			"		apply sample to " PATH_RADVD_CONF "\n\n"
			"	-apply-option\n"
			"	-ao\n"
			"		apply options to " PATH_RADVD_CONF "\n\n"
			"	-option-reset\n"
			"	-or\n"
			"		reset options to default\n\n"
			"	-option-dump\n"
			"	-od\n"
			"		tdump options\n\n"
			"	name=value\n"
			"		set a option\n"
			"		e.g. interface.enable=1\n\n"
			"	-radvd-stop\n"
			"	-rs\n"
			"		stop radvd\n\n"
			"	-radvd-reload\n"
			"	-rr\n"
			"		make radvd reload conf file\n\n"
	);
}

int
radvdconf_main(int argc, char *argv[])
{
	int i;
	int apply_sample = 0;
	int apply_option = 0;
	int option_default = 0;
	int option_dump = 0;
	int option_set = 0;
	int radvd_stop = 0;
	int radvd_reload = 0;
	
	//printf( "radvdconf start\n" );
	
	for( i = 0; i < argc; i ++ ) {
		//printf( "\t%d: %s\n", i, argv[ i ] );
		
		if( strcmp( argv[ i ], "-apply-sample" ) == 0 ||
			strcmp( argv[ i ], "-as" ) == 0 )
		{
			apply_sample = 1;
		} else if(  strcmp( argv[ i ], "-apply-option" ) == 0 ||
					strcmp( argv[ i ], "-ao" ) == 0 )
		{
			apply_option = 1;
		} else if(  strcmp( argv[ i ], "-option-reset" ) == 0 ||
					strcmp( argv[ i ], "-or" ) == 0 )
		{
			option_default = 1;
		} else if(  strcmp( argv[ i ], "-option-dump" ) == 0 ||
					strcmp( argv[ i ], "-od" ) == 0 )
		{
			option_dump = 1;
		} else if(  strcmp( argv[ i ], "-radvd-stop" ) == 0 ||
					strcmp( argv[ i ], "-rs" ) == 0 )
		{
			radvd_stop = 1;
		} else if(  strcmp( argv[ i ], "-radvd-reload" ) == 0 ||
					strcmp( argv[ i ], "-rr" ) == 0 )
		{
			radvd_reload = 1;
		}
	}

	// default options 
	if( option_default ) {
		printf( "default options\n" );
		options_default();
	}
	
	// parse options in arguments 
	for( i = 0; i < argc; i ++ ) {
		if( parse_option( argv[ i ] ) ) {
			printf( "Set Option: %s\n", argv[ i ] );
			option_set ++;
		}
	}
	
	// dump options  
	if( option_dump ) {
		printf( "dump options \n" );
		options_dump();
	}
	
	// apply sample to conf file  
	if( apply_sample ) {
		printf( "write radvd conf sample\n" );
		write_conf_sample();
	}
	
	// apply option to conf file 
	if( apply_option ) {
		printf( "write options to radvd conf \n" );
		write_conf_options();
	}
	
	// radvd stop 
	if( radvd_stop ) {
		
		// This is a very hack coding 
		extern volatile int sigterm_received;
		extern cyg_handle_t radvd_thread_handle;
		cyg_thread_info info;
		char *state_string;
		cyg_uint16 id = 0;
		memset(&conf,0,sizeof(conf));
		printf( "radvd is going to stop!\n" );
		if(radvd_thread_handle!=NULL)
		{
			if((id=cyg_thread_get_id(radvd_thread_handle))>0)
			{
				cyg_thread_get_info( radvd_thread_handle, id, &info );
				if(info.state & 0x1b != 0x10)
				{
					sigterm_received = 1;
				}
			}
		}
	}
	
	// radvd reload 
	if( radvd_reload ) {
	
		/// This is a very hack coding 
		extern volatile int sighup_received;
		
		printf( "radvd is going to reload!\n" );
		sighup_received = 1;
	}
	
	// show usage 
	if( !option_default && !option_dump && !apply_sample && !apply_option && 
		!option_set &&
		!radvd_stop && !radvd_reload ) 
	{
		usage();
	}
	
	return 0;
}

