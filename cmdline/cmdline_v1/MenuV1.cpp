#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "utils/football_debugger.h"

#include "CmdLine.h"

#include "linktable.h"

#include "MenuV1.h"

#undef __CLASS__
#define __CLASS__ "MenuV1"

#if 0

#undef JNI_FR_DEBUG_H_
#if 1  // must @ last
#undef config_fr_DEBUG_EN
#undef config_fr_DEBUG_TO_CONSOLE
#define config_fr_DEBUG_EN 1  //  (1)
#define config_fr_DEBUG_TO_CONSOLE 1 //  1 //  0
#include "cycling_utils/cy_debug.h"
#endif
#undef LOG_TAG
#define LOG_TAG "MenuV1"

#endif


#define MENUV1_CMD_MAX_LEN 		2048 // 1024
#define MENUV1_CMD_MAX_ARGV_NUM 	64 // 32

namespace NS_cmdline_v1 {
namespace NS_menu_v1 {

#if 1

/* data struct and its operations */
typedef struct DataNode {
    tLinkTableNode * pNext;
    const char*   cmd;
    const char*   desc;
	void *ctx;
    PF_handler handler;
} tDataNode;

static int SearchConditon(tLinkTableNode * pLinkTableNode,void * arg) {
    char * cmd = (char*)arg;
    tDataNode * pNode = (tDataNode *)pLinkTableNode;
    if(strcmp(pNode->cmd, cmd) == 0) {
        return  SUCCESS;  
    }
    return FAILURE;	       
}
/* find a cmd in the linklist and return the datanode pointer */
static tDataNode* FindCmd(tLinkTable * head, char * cmd) {
    tDataNode * pNode = (tDataNode*)GetLinkTableHead(head);
    while(pNode != NULL) {
        if(!strcmp(pNode->cmd, cmd))
        {
            return  pNode;  
        }
        pNode = (tDataNode*)GetNextLinkTableNode(head,(tLinkTableNode *)pNode);
    }
    return NULL;
}

/* show all cmd in listlist */
static int ShowAllCmd(tLinkTable * head) {
    tDataNode * pNode = (tDataNode*)GetLinkTableHead(head);
    while(pNode != NULL) {
        fprintf(stderr, "    * %s - %s\n", pNode->cmd, pNode->desc);
        pNode = (tDataNode*)GetNextLinkTableNode(head,(tLinkTableNode *)pNode);
    }
    return 0;
}

/*static*/ int MenuV1::Help(void *ctx, int argc, char * const argv[]) {
	//FrLOGV(LOG_TAG, "%s,%d ", __func__, __LINE__);
	MenuV1 *menu_ = (MenuV1*)ctx;
	ShowAllCmd((LinkTable *)menu_->mHead);
	return 0;
}

MenuV1::MenuV1() {
	//FrLOGV(LOG_TAG, "%s,%d ", __func__, __LINE__);
	mPrompt = (char *)malloc(MENUV1_CMD_MAX_LEN + 4);
	memset(mPrompt, 0, MENUV1_CMD_MAX_LEN + 4);
	mHead = NULL;
}
MenuV1::~MenuV1() {
	// free list
	if (mHead != NULL) {
		LinkTable *head = (LinkTable *)mHead;
		DeleteLinkTable(head);
		mHead = NULL;
	}
	// free prompt
	if (mPrompt != NULL) {
		free(mPrompt);
		mPrompt = NULL;
	}
	//FrLOGV(LOG_TAG, "%s,%d ", __func__, __LINE__);
}

/* Set Input Prompt */
int MenuV1::setPrompt(const char * p) {
    if (p == NULL) {
        return 0;
    }
    strcpy(mPrompt,p);
	return 0;
}
/* add cmd to menu */
int MenuV1::menuConfig(const char * cmd, const char * desc, PF_handler handler, void *ctx) {
    tDataNode* pNode = NULL;
	LinkTable *head = (LinkTable *)mHead;
    if ( head == NULL) {
		head = CreateLinkTable();
		mHead = head;
	
		pNode = (tDataNode*)malloc(sizeof(tDataNode));
		pNode->cmd = android::Cmdline::kCmd_help;
		pNode->desc = "Menu List";
		pNode->handler = Help;
		pNode->ctx = this;
		AddLinkTableNode(head,(tLinkTableNode *)pNode);

		pNode = (tDataNode*)malloc(sizeof(tDataNode));
		pNode->cmd = android::Cmdline::kCmd_h_;
		pNode->desc = "Menu List";
		pNode->handler = Help;
		pNode->ctx = this;
		AddLinkTableNode(head,(tLinkTableNode *)pNode);
		
    }
    pNode = (tDataNode*)malloc(sizeof(tDataNode));
    pNode->cmd = cmd;
    pNode->desc = desc;
    pNode->handler = handler; 
	pNode->ctx = ctx;
    AddLinkTableNode(head,(tLinkTableNode *)pNode);
    return 0; 
}

/* Menu Engine Execute */
int MenuV1::loop() {
   /* cmd line begins */
	
    while(1) {
		int argc = 0;
		char *argv[MENUV1_CMD_MAX_ARGV_NUM];
        char cmd[MENUV1_CMD_MAX_LEN + 16];
		char *pcmd = NULL;
        DLOGD("%s", mPrompt);

		// get one line from cmline
		
		/* scanf("%s", cmd); */
#if 1  
		pcmd = fgets(cmd, MENUV1_CMD_MAX_LEN, stdin);
#endif

		if(pcmd == NULL) {
			continue;
		}
	#if 1
		//DLOGD( "(%s)\r\n", pcmd);
		int pcmd_length = strlen(pcmd);
		if (pcmd_length >= 1) {
			if (pcmd[pcmd_length - 1] == '\n') {
				pcmd[pcmd_length - 1] = ' ';
			}
			else if (pcmd[pcmd_length - 1] == '\r') {
				pcmd[pcmd_length - 1] = ' ';
			}
		}
		else if (pcmd_length >= 2) {
			if (pcmd[pcmd_length - 2] == '\r'
				&& pcmd[pcmd_length - 1] == '\n') {
				pcmd[pcmd_length - 2] = ' ';
				pcmd[pcmd_length - 1] = ' ';
			}
			else if (pcmd[pcmd_length - 2] == '\n'
				&& pcmd[pcmd_length - 1] == '\r'
				) {
				pcmd[pcmd_length - 2] = ' ';
				pcmd[pcmd_length - 1] = ' ';
			}
		}
		if (strlen(pcmd) == 0) {
			continue;
		}
		//DLOGD( "(%s)\r\n", pcmd);
	#endif
	
        /* convert cmd to argc/argv */
		pcmd = strtok(pcmd," ");
		while(pcmd != NULL && argc < MENUV1_CMD_MAX_ARGV_NUM) {
			argv[argc] = pcmd;
			argc++;
			pcmd = strtok(NULL," ");
		}
	#if 0
        if(argc == 1) {
            int len = strlen(argv[0]);
            *(argv[0] + len - 1) = '\0';
        }
	#endif
		//DLOGD( "argc:%d argv[0]=(%s)\r\n", argc, argv[0]);
	
		if (argc == 0 || argv[0] == NULL || strlen(argv[0]) <= 0) {
			continue;
		}

		int proc_ret = cmd_process(argc, argv);
		if (proc_ret == -10000) {
			break;
		}
    }

	return 0;
}

int MenuV1::cmd_process(int argc, char * const argv[]) {
	LinkTable *head = (LinkTable *)mHead;
	tDataNode *p = (tDataNode*)SearchLinkTableNode(head,SearchConditon,(void*)argv[0]);
	if( p == NULL) {
		//DLOGD( "%s,%d no command\r\n", __func__, __LINE__);
		return 0;
	}
	DLOGD("%s - %s\n", p->cmd, p->desc);
	if(p->handler != NULL) { 
		int r = p->handler(p->ctx, argc, argv);
		return r;
	}
	return -1;
}

#endif


};  // namespace NS_menu_v1 {
};  // namespace NS_cmdline_v1 {

