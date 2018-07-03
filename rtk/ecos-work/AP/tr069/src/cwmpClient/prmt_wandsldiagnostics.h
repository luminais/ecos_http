#ifndef _PRMT_WANDSLDIAGNOSTICS_H_
#define _PRMT_WANDSLDIAGNOSTICS_H_

#include "prmt_igd.h"

#ifdef __cplusplus
extern "C" {
#endif


extern struct CWMP_LEAF tWANDSLDIAGLeaf[];

int getWANDSLDIAG(char *name, struct CWMP_LEAF *entity, int *type, void **data);
int setWANDSLDIAG(char *name, struct CWMP_LEAF *entity, int type, void *data);

extern int gStartDSLDiag;
void cwmpStartDSLDiag(void);



enum
{
	GET_DSL_ACTPSDds=0,
	GET_DSL_ACTPSDus,
	GET_DSL_ACTATPds,
	GET_DSL_ACTATPus,
	
	GET_DSL_HLINpsds,
	GET_DSL_HLINpsus,
	GET_DSL_HLINSCds,
	GET_DSL_HLINSCus,

	GET_DSL_HLOGpsds,
	GET_DSL_HLOGpsus,
	GET_DSL_HLOGMTds,
	GET_DSL_HLOGMTus,

	GET_DSL_QLNpsds,
	GET_DSL_QLNpsus,
	GET_DSL_QLNMTds,
	GET_DSL_QLNMTus,

	GET_DSL_SNRpsds,
	GET_DSL_SNRpsus,
	GET_DSL_SNRMTds,
	GET_DSL_SNRMTus,
	
	GET_DSL_LATNds,
	GET_DSL_LATNus,
	GET_DSL_SATNds,
	GET_DSL_SATNus,
	
	GET_DSL_BITSpsds,
	GET_DSL_GAINSpsds,
	
	DSL_END
};
int getDSLParameterValue( unsigned int name_idx, void *pvalue );




#ifdef __cplusplus
}
#endif

#endif /*_PRMT_WANDSLDIAGNOSTICS_H_*/
