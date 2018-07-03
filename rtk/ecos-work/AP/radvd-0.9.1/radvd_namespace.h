#ifndef __RADVD_NAMESPACE_H__
#define __RADVD_NAMESPACE_H__

// ---------------------------------------------------------------------------
// This header file is used to rename radvd's variables and functions to 
// avoid name conflict. 
// 
// Our rules are: 
//   1. make external symbols to become 'static' (modify whthin .c files)
//   2. add 'radvd_' prefix (topic of this file)
// 

// ---------------------------------------------------------------------------
// variables 
#if 0
/* scanner.l */
#define num_lines			radvd_num_lines
#define yyin				radvd_yyin
#define yytext				radvd_yytext

/* gram.c */
#define yylval				radvd_yylval
#endif

/* radvd.c */
#define IfaceList			radvd_IfaceList
#define sighup_received		radvd_sighup_received
#define sigterm_received	radvd_sigterm_received
#define conf_file			radvd_conf_file
#define sock				radvd_sock

// ---------------------------------------------------------------------------
// functions (names will be the same as radvd.h)

/* gram.y */
#define yyparse( void )		radvd_yyparse( void )	

/* scanner.l */
#define yylex				radvd_yylex		// with () due to 'scanner.c' syntax 

/* radvd.c */
#define check_ip6_forwarding( void )		radvd_check_ip6_forwarding( void )

/* timer.c */
#define set_timer( tm, t )			radvd_set_timer( tm, t )
#define clear_timer( tm )			radvd_clear_timer( tm )
#define init_timer( tl, f, d )		radvd_init_timer( tl, f, d )

/* log.c */
#define log_open( m, i, l, f)		radvd_log_open( m, i, l, f )
#define flog( p, s... )				radvd_flog( p, s )
#define dlog( p, l, s... )			radvd_dlog( p, l, s )
#define log_close( void )			radvd_log_close( void )
#define log_reopen( void )			radvd_log_reopen( void )
#define set_debuglevel( l )			radvd_set_debuglevel( l )
#define get_debuglevel( void )		radvd_get_debuglevel( void )

/* device.c */
#define setup_deviceinfo( sock, ifs )				radvd_setup_deviceinfo( sock, ifs )
#define check_device( sock, ifs )					radvd_check_device( sock, ifs )
#define setup_linklocal_addr( sock, ifs )			radvd_setup_linklocal_addr( sock, ifs )
#define setup_allrouters_membership( sock, ifs )	radvd_setup_allrouters_membership( sock, ifs )
#define check_allrouters_membership( sock, ifs )	radvd_check_allrouters_membership( sock, ifs )
#define get_v4addr( ifn, dst )						radvd_get_v4addr( ifn, dst )
#define set_interface_linkmtu( ifn, n )				radvd_set_interface_linkmtu( ifn, n )
#define set_interface_curhlim( ifn, n )				radvd_set_interface_curhlim( ifn, n )
#define set_interface_reachtime( ifn, n )			radvd_set_interface_reachtime( ifn, n )
#define set_interface_retranstimer( ifn, n )		radvd_set_interface_retranstimer( ifn, n )

/* interface.c */
#define iface_init_defaults( ifn )			radvd_iface_init_defaults( ifn )
#define prefix_init_defaults( ap )			radvd_prefix_init_defaults( ap )
#define route_init_defaults( ar, ifs )		radvd_route_init_defaults( ar, ifs )
#define check_iface( ifs )					radvd_check_iface( ifs )

/* socket.c */
#define open_icmpv6_socket( void )			radvd_open_icmpv6_socket( void )
#define set_ipv6_mreq_allrouters( s, i, o )	radvd_set_ipv6_mreq_allrouters( s, i, o )	

/* send.c */
#define send_ra( s, i, d )					radvd_send_ra( s, i, d )

/* process.c */
#define process( s, i, m, l, a, p, h )		radvd_process( s, i, m, l, a, p, h )

/* recv.c */
#define recv_rs_ra( s, m, a, p, h )			radvd_recv_rs_ra( s, m, a, p, h )

/* util.c */
#define mdelay( n )					radvd_mdelay( n )
#define rand_between( n, m )		radvd_rand_between( n, m )
#define print_addr( a, buf )		radvd_print_addr( a, buf )

#define yynerrs			radvd_yynerrs
#define yychar			radvd_yychar
#define yylval			radvd_yylval
#define yy_flush_buffer		radvd_yy_flush_buffer
#define yytext			radvd_yytext
#define yyin			radvd_yyin
#define yyget_lineno		radvd_yyget_lineno
#define yyget_in		radvd_yyget_in
#define yyget_out		radvd_yyget_out
#define yyout			radvd_yyout
#define yyget_leng		radvd_yyget_leng
#define yyleng			radvd_yyleng
#define yyget_text		radvd_yyget_text
#define yyset_lineno		radvd_yyset_lineno
#define yyset_in		radvd_yyset_in
#define yyset_out		radvd_yyset_out
#define yyget_debug		radvd_yyget_debug
#define yy_flex_debug		radvd_yy_flex_debug
#define yyset_debug		radvd_yyset_debug
#define yyfree			radvd_yyfree
#define yy_delete_buffer	radvd_yy_delete_buffer
#define yypop_buffer_state	radvd_yypop_buffer_state
#define yylex_destroy		radvd_yylex_destroy
#define yyrealloc		radvd_yyrealloc
#define yyalloc			radvd_yyalloc
#define yypush_buffer_state	radvd_yypush_buffer_state
#define yy_switch_to_buffer	radvd_yy_switch_to_buffer
#define yy_scan_buffer		radvd_yy_scan_buffer
#define yy_scan_bytes		radvd_yy_scan_bytes
#define yy_scan_string		radvd_yy_scan_string
#define yylineno		radvd_yylineno
#define yy_create_buffer	radvd_yy_create_buffer
#define yyrestart		radvd_yyrestart
#endif // __RADVD_NAMESPACE_H__

