#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#define LINE_LEN 500
#define DEMENSION_NUM 23
#define AFFAIR_NUM 8124
#define DATA_FILE "mushroom"
#define ITEM_NUM 120
#define MAX_ITEM_NUM 10
#define MAX_FULL_TREE_SIZE 30000
#define MAX_COND_TREE_SIZE 5000
#define MIN_SUPPORT_PERCENT 0.25
#define MIN_SUPPORT_NUM MIN_SUPPORT_PERCENT * AFFAIR_NUM
#define min(a,b) (((a) < (b)) ? (a) : (b))
#define max(a,b) (((a) > (b)) ? (a) : (b))
#define TRUE 1
#define FALSE 0

typedef unsigned char bool;

typedef struct fpNode
{
    int name;
    int support;
    int parent;//指示fpTree中父节点
    struct fpNode *p;//指向同名节点
}fpNode;

typedef struct
{
    fpNode *nodes;
    fpNode headerTab[ITEM_NUM];//头表与树绑定
    int headerTabNum;//头表中有效项个数
    int n;//节点数
}fp_tree;//

typedef struct
{
    int nodes[ITEM_NUM];
    int n;
    int support;
}fp_list;//


FILE *fp;
char *tonken, *saveptr, *tmpline, *tmptoken;

int data[AFFAIR_NUM][DEMENSION_NUM];

fpNode fpData[AFFAIR_NUM][DEMENSION_NUM];
fpNode branchData[AFFAIR_NUM][ITEM_NUM];//记录条件fp树路径信息
fp_tree fpTree;
fpNode result[ITEM_NUM];
int pos;
int itemCount;

//读入数据
void parse()
{
    char *readLine = (char *)malloc(sizeof(char) * LINE_LEN);
    fp = fopen(DATA_FILE, "r");
    for(int i = 0; i < AFFAIR_NUM && fgets(readLine, LINE_LEN, fp) != NULL; i++)
    {
	tmpline = readLine;
	tmptoken = tonken;
	for(int j = 0; j < DEMENSION_NUM; j++, tmpline = NULL )
	    if((tmptoken = strtok_r(tmpline, " ", &saveptr)) == NULL)
		break;
	    else
		sscanf(tmptoken, "%d", &data[i][j]);
    }
    free(readLine);
}

void getHeaderTab()
{

    fpTree.nodes = (fpNode*)malloc(sizeof(fpNode) * MAX_FULL_TREE_SIZE);
    fpTree.n = 1;
    fpTree.nodes[0].name = -1;
    fpTree.nodes[0].parent = -1;

    int n = 0;
    fpNode swap;
    for(int j = 0; j < DEMENSION_NUM; j++)
	for(int i = 0; i < AFFAIR_NUM; i++)
	{
	    int k;
	    for(k = 0; k < n && data[i][j] != 0; k++)
	    {
		if(fpTree.headerTab[k].name == data[i][j])//若有记录
		{
		    fpTree.headerTab[k].support++;
		    break;
		}
	    }
	    if(k >= n && data[i][j] != 0)//无记录
	    {
		fpTree.headerTab[n].name = data[i][j];
		fpTree.headerTab[n].support++;
		n++;
	    }
	}

//按支持度从大到小排序
    for(int i = 0; i < ITEM_NUM; i++)
	for(int j = i; j < ITEM_NUM; j++)
	    if(fpTree.headerTab[i].support < fpTree.headerTab[j].support)
	    {
		swap = fpTree.headerTab[i];
		fpTree.headerTab[i] = fpTree.headerTab[j];
		fpTree.headerTab[j] = swap;
	    }

    fpTree.headerTabNum = 0;//找出头表有效项个数
    for(int i = 0; i < ITEM_NUM && fpTree.headerTab[i].support >= MIN_SUPPORT_NUM; i++)
    {
	fpTree.headerTabNum++;
    }
}

//按支持度从高到低重排每一个事务的项集,去除达不到阈值的项
void initData()
{
    for(int i = 0; i < AFFAIR_NUM; i++)
    {
	int count = 0;
	for(int k = 0; k < ITEM_NUM && fpTree.headerTab[k].support >= MIN_SUPPORT_NUM; k++)
	    for(int j = 0; j < DEMENSION_NUM && data[i][j] != 0; j++)
		if(data[i][j] == fpTree.headerTab[k].name) //根据维度对比取值
		    fpData[i][count++].name = fpTree.headerTab[k].name;
    }
}

void getFpTree()
{
    int currentParent;
    for(int i = 0; i < AFFAIR_NUM; i++)
    {
	currentParent = 0;//每个事务均从根节点开始
	for(int j = 0; j < DEMENSION_NUM && fpData[i][j].name != 0; j++)//name == 0 表明此项已删除
	{
	    int k;
	    //若路径重叠,则遍历找子节点,经过的节点计数加一
	    for(k = 1; k < fpTree.n; k++)
		if(fpTree.nodes[k].name == fpData[i][j].name && fpTree.nodes[k].parent == currentParent)
		{
		    fpTree.nodes[k].support++;
		    currentParent = k;
		    break;
		}

	    //加一个新节点
	    if(k >= fpTree.n)
	    {
		fpTree.nodes[fpTree.n].name = fpData[i][j].name;
		fpTree.nodes[fpTree.n].parent = currentParent;
		fpTree.nodes[fpTree.n].support = 1;
		currentParent = fpTree.n;
		fpTree.n++;
	    }
	}
    }

//    for(int i = 0; i < fpTree.n; i++)
//	printf("name %d sup %d i %d\n", fpTree.nodes[i].name, fpTree.nodes[i].support, i);
}

//把headerTab,fpTree中同名节点建立链接
void link()
{
    for(int i = 0; i < ITEM_NUM; i++)
	fpTree.headerTab[i].p = NULL;
    for(int i = 0; i < fpTree.n; i++)
	fpTree.nodes[i].p = NULL;

    //headerTab指向fp树中第一个同名节点
    for(int i = 0; i < ITEM_NUM; i++)
	for(int j = 0; j < fpTree.n; j++)
	    if(fpTree.headerTab[i].name == fpTree.nodes[j].name)
	    {
		fpTree.headerTab[i].p = &fpTree.nodes[j];
		break;
	    }

    //fp树各节点指向最近的同名节点
    for(int i = 1; i < fpTree.n - 1; i++)
	for(int j = i + 1; j < fpTree.n; j++)
	    if(fpTree.nodes[i].name == fpTree.nodes[j].name)
	    {
		fpTree.nodes[i].p = &fpTree.nodes[j];
		break;
	    }
}

int getParent(fpNode *child)
{
    return child->parent;
}

void mergeBranch(fp_tree *condTree, fp_tree *branch)
{
    int parent = 0;//tree中当前节点的父节点标号
    int len = 0;//重叠路径的长度
    int i;
    for(int loop = 1; loop < branch->n; loop++)
    {
	for(i = 1; i < condTree->n; i++)
	{
	    if(condTree->nodes[i].parent == parent && branch->nodes[branch->n - len - 1].name == condTree->nodes[i].name)//找到父节点的各子节点
	    {
		condTree->nodes[i].support += branch->nodes[branch->n - len - 1].support;
		len++;
		parent = i;
		break;
	    }
	}

	if(i >= condTree->n)//未找到匹配节点
	{
	    for(int j = branch->n - len - 1; j > 0; j--)//把brach中剩余节点插入tree中
	    {
		condTree->nodes[condTree->n].parent = parent;
		condTree->nodes[condTree->n].name = branch->nodes[j].name;
		condTree->nodes[condTree->n].support = branch->nodes[j].support;
		parent = condTree->n;
		condTree->n++;
	    }
	    break;
	}
    }
//    printf("\n");
/*    for(int i = 1; i < branch->n; i++)
      printf("branch name %d parent %d i %d\n", branch->nodes[i].name, branch->nodes[i].parent, i);
      printf("\n");
      for(int i = 1; i < condTree->n; i++)
      printf("name %d parent %d i %d\n", condTree->nodes[i].name, condTree->nodes[i].parent, i);
      printf("\n");
*/
}


void insert(fp_tree *tree, fpNode *insertNode, int addSup)
{
    tree->n++;
    tree->nodes[tree->n - 1].name = insertNode->name;
    tree->nodes[tree->n - 1].support = addSup;//新加的节点,支持度取决于起始点
    if(insertNode->parent != 0)
	tree->nodes[tree->n - 1].parent = tree->n;
}

void addToCondBranch(fpNode *node, fp_tree *branch, int addSup)
{
    branch->n++;
    branch->nodes[branch->n - 1].name = node->name;
    branch->nodes[branch->n - 1].support = addSup;//新加的节点,支持度取决于起始点
    if(node->parent != 0)
	branch->nodes[branch->n - 1].parent = branch->n;
}

//删掉树中第i个结点
void removeNode(fp_tree *tree, int num)
{
    if(num == tree->n - 1)
	tree->n--;
    else if(num >= 0 && num < tree->n - 1)
    {
	for(int i = num; i < tree->n - 1; i++)
	    tree->nodes[i] = tree->nodes[i + 1];
	tree->n--;
    }
}

//从树中任取num个结点的所有组合
void getCombPattern(fp_tree tree, fp_list list, int minSupport, int begin, int num)//从第begin个元素开始,找num个结点的任意组合
{
    if(num == 0)
    {
	for(int i = pos - 1; i >= 0; i--)
	{
	    minSupport = min(minSupport, result[i].support);
	    printf("| %d ", result[i].name);		
	}
	for(int i = list.n - 1; i >= 0; i--)
	    printf("| %d ", list.nodes[i]);		
	printf("| : %d\n", minSupport);
    }
    else 
	for(int i = begin; i < tree.n - num + 1; i++)
	{
	    result[pos].support = tree.nodes[i].support;
	    result[pos++].name = tree.nodes[i].name;
	    getCombPattern(tree, list, minSupport, i + 1, num - 1);
	    pos--;
	}
}

//fp-growth
//用于挖掘条件fp树(单链,节省空间)中的频繁模式
void fpCondGrowth(fp_tree condTree, fp_list list)
{
//    for(int i = 1; i < condTree.n; i++)
//	printf("name %d sup %d i %d parent %d\n", condTree.nodes[i].name, condTree.nodes[i].support, i, condTree.nodes[i].parent);
//    
    for(int i = 1; i < condTree.n; i++)//删除未达到阈值的节点
	if(condTree.nodes[i].support < MIN_SUPPORT_NUM)
	{
	    removeNode(&condTree, i);
	    i = 0;
	}

    if(condTree.n > 1)//删除节点后树不为空
    {
	//挖掘此链上所有频繁模式
	for(int i = condTree.n - 1; i > 0; i--)
	{
	    pos = 0;
	    getCombPattern(condTree, list, AFFAIR_NUM, 1, i);		
	}
    }
}

void getCondTree(fp_tree *traceTree, fp_tree *condTree, fpNode *node)
{
    int addSup;
    fpNode *trace;
    fp_tree branch;
    branch.nodes = (fpNode*)malloc(sizeof(fpNode) * MAX_COND_TREE_SIZE);
    branch.nodes[0].name = -1;
    branch.nodes[0].parent = -1;
    while(node != NULL)
    {
	addSup = node->support;
	if(node->parent == 0)
	{
	    node = node->p;
	    continue;
	}
	branch.n = 1;
	trace = &traceTree->nodes[node->parent];
	while(trace->parent != -1)
	{
	    addToCondBranch(trace, &branch, addSup);//把路径上的节点加入条件fp树 
	    trace = &traceTree->nodes[trace->parent]; 
	}
	branch.nodes[branch.n - 1].parent = 0;//指回原节点
//	for(int i = 1; i < branch.n; i++)
//	    printf("name %d sup %d i %d parent %d\n", branch.nodes[i].name, branch.nodes[i].support, i, branch.nodes[i].parent);
//	for(int i = 1; i < branch.n; i++)
//	    printf("name %d sup %d ", branch.nodes[i].name, branch.nodes[i].support);
///	printf("\n");
	mergeBranch(condTree, &branch);
	node = node->p;//取下一个同名节点 
    }
//    for(int i = 0; i < condTree->n; i++)
//	printf("name : %d sup %d parent %d\n", condTree->nodes[i].name, condTree->nodes[i].support, condTree->nodes[i].parent);
    getCondHeaderTab(condTree);//建立新表头
    printf("headerTab %d \n", condTree->headerTabNum);
    for(int i = 0; i < condTree->headerTabNum; i++)
	printf("name : %d sup %d i %d\n", condTree->headerTab[i].name, condTree->headerTab[i].support, i);

    linkCondNode(condTree);//建立链接
}
void addToList(fp_list *list, fpNode *node)
{
    fpNode *tmp = node;
    int sumSup = 0;
    if(node != NULL)
    {
	while(tmp != NULL)
	{
	    sumSup += tmp->support;
	    tmp = tmp->p;
	}

	list->nodes[list->n] = node->name;

	list->support = sumSup;
	list->n++;	    		
    }
    else
	printf("NULL node\n");

}

//对整个fp树进行处理,将每个分支分解
void fpGrowth(fp_tree *tree, fp_list list)//, int num)//取headerTab中num号数据
{
    bool isSingle = TRUE;
    fp_tree condTree;
    fpNode *tmpNode;//用于遍历同名节点 
    fpNode *traceNode;//用于回溯指定节点 

    for(int i = 1; i < tree->n - 1; i++)//判断是否单链
	for(int j = i + 1; j < tree->n; j++)
	    if(tree->nodes[i].parent == tree->nodes[j].parent)
	    {
		isSingle = FALSE;
		break;
	    }

    if(isSingle)//单链可直接输出频繁项集
	fpCondGrowth(*tree, list);

    else //多链需进一步分解
    {

	condTree.nodes = (fpNode* )malloc(sizeof(fpNode) * MAX_COND_TREE_SIZE);
	condTree.nodes[0].name = -1;
	condTree.nodes[0].parent = -1;
	int num = tree->headerTabNum - 1;
	while(num--)
	{

	    condTree.n = 1;
	    memset(condTree.headerTab, 0, sizeof(condTree.headerTab));
	    condTree.headerTabNum = 0;
//	    printf("hello\n");
//	    printf("headertab %d\n",  tree->headerTab[num].p);
	    addToList(&list, tree->headerTab[num].p);
//	    printf("tree.n %d\n", tree->n);

	    getCondTree(tree, &condTree, tree->headerTab[num].p);
//	    for(int i = 0; i < condTree.n; i++)
//		printf("name %d sup %d i %d parent %d\n", condTree.nodes[i].name, condTree.nodes[i].support, i, condTree.nodes[i].parent);

//	    printf("num %d\n", num);
//	    printf("\n");
	    if(condTree.n > 1)
		fpGrowth(&condTree, list);	    		

//	    }
	}
    }

}

int main()
{
    fp_list list;
    memset(list.nodes, 0 ,sizeof(list.nodes));
    list.n = 0;
    list.support = AFFAIR_NUM;
    parse();
    getHeaderTab();
    initData();
    getFpTree();
    link();
    fpGrowth(&fpTree, list);

    return 0;
}

