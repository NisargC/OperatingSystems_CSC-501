#include <stdio.h>

extern int etext, edata;

void printsegaddress() {

	printf("\nvoid printsegaddress()\n\n");
	printf("Current: etext[0x%08x]=0x%08x, edata[0x%08x]=0x%08x, ebss[0x%08x]=0x%08x\n", &etext, etext, &edata, edata, &end, end);
	printf("Preceeding: etext[0x%08x]=0x%08x, edata[0x%08x]=0x%08x, ebss[0x%08x]=0x%08x\n", &etext-1, *(&etext-1), &edata-1, *(&edata-1), &end-1, *(&end-1));
        printf("After: etext[0x%08x]=0x%08x, edata[0x%08x]=0x%08x, ebss[0x%08x]=0x%08x\n", &etext+1, *(&etext+1), &edata+1, *(&edata+1), &end+1, *(&end+1));

	return;
}
