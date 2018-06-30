#include <bcmnvram.h>

void	backupConfig(void);
void	restoreConfig(void);
 int restoreNvramValue(struct nvram_header *header_backupcfg);

int 	checkIfNeedBackupcfg(void);
void	backupTimer();
int checkIfNeedRestorecfg(void);

