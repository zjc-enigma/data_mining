#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define LINE_LEN 500
#define DEMENSION_NUM 50
#define AFFAIR_NUM 340183
#define DATA_FILE "accidents.dat"
#define ITEM_NUM 468
#define MAX_ITEM_NUM 10
#define MAX_FULL_TREE_SIZE 1000000
#define MAX_COND_TREE_SIZE 468
#define MIN_SUPPORT_PERCENT 0.4
#define MIN_SUPPORT_NUM MIN_SUPPORT_PERCENT * AFFAIR_NUM
#define min(a,b) (((a) < (b)) ? (a) : (b))
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
    fpNode nodes[MAX_FULL_TREE_SIZE];
    int r;//根
    int n;//节点数
}fpGrowthTree;//完整fp树

typedef struct
{
    fpNode nodes[MAX_COND_TREE_SIZE];
    int r;//根
    int n;//节点数
}fpCondTree;//条件fp树,远小于完整fp树


FILE *fp;
char *tonken, *saveptr, *tmpline, *tmptoken;

int data[AFFAIR_NUM][DEMENSION_NUM];

fpNode headerTab[ITEM_NUM];
int headerTabNum;//头表中有效项个数

fpNode fpData[AFFAIR_NUM][DEMENSION_NUM];
fpGrowthTree fpTree;
fpNode result[ITEM_NUM];
int pos;


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
    int n = 0;
    fpNode swap;
    for(int j = 0; j < DEMENSION_NUM; j++)
	for(int i = 0; i < AFFAIR_NUM; i++)
	{
	    int k;
	    for(k = 0; k < n && data[i][j] != 0; k++)
	    {
		if(headerTab[k].name == data[i][j])//若有记录
		{
		    headerTab[k].support++;
		    break;
		}
	    }
	    if(k >= n && data[i][j] != 0)//无记录
	    {
		headerTab[n].name = data[i][j];
		headerTab[n].support++;
		n++;
	    }
	}

//按支持度从大到小排序
    for(int i = 0; i < ITEM_NUM; i++)
	for(int j = i; j < ITEM_NUM; j++)
	    if(headerTab[i].support < headerTab[j].support)
	    {
		swap = headerTab[i];
		headerTab[i] = headerTab[j];
		headerTab[j] = swap;
	    }

    headerTabNum = 0;//找出头表有效项个数
    for(int i = 0; i < ITEM_NUM && headerTab[i].support >= MIN_SUPPORT_NUM; i++)
	headerTabNum++;
    
}

//按支持度从高到低重排每一个事务的项集,去除达不到阈值的项
void initData()
{
    for(int i = 0; i < AFFAIR_NUM; i++)
    {
	int count = 0;
	for(int k = 0; k < ITEM_NUM && headerTab[k].support >= MIN_SUPPORT_NUM; k++)
	    for(int j = 0; j < DEMENSION_NUM && data[i][j] != 0; j++)
		if(data[i][j] == headerTab[k].name)
		    fpData[i][count++].name = headerTab[k].name;
    }
}

void getFpTree()
{
    fpTree.n = 1;
    fpTree.r = 0;
    fpTree.nodes[0].name = -1;
    fpTree.nodes[0].parent = -1;
    int currentParent;
    bool B;
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
	    if(k >= fpTree.n)// || B == FALSE)
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
	headerTab[i].p = NULL;
    for(int i = 0; i < fpTree.n; i++)
	fpTree.nodes[i].p = NULL;

    //headerTab指向fp树中第一个同名节点
    for(int i = 0; i < ITEM_NUM; i++)
	for(int j = 0; j < fpTree.n; j++)
	    if(headerTab[i].name == fpTree.nodes[j].name)
	    {
		headerTab[i].p = &fpTree.nodes[j];
		break;
	    }

    //fp树各节点指向最近的同名节点
    for(int i = 0; i < fpTree.n - 1; i++)
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

void insert(fpCondTree *tree, fpNode *insertNode, int addSup)
{
    tree->n++;
    tree->nodes[tree->n - 1].name = insertNode->name;
    tree->nodes[tree->n - 1].support = addSup;//新加的节点,支持度取决于起始点
}

void addToCond(fpCondTree *tree, fpNode *node, int addSup)
{
    int i;
    for(i = 0; i < tree->n; i++)
	if(node->name == tree->nodes[i].name)
	{
	    tree->nodes[i].support += addSup;
	    break;
	}
    if(i >= tree->n)//没有匹配的节点,则插入新节点
	insert(tree, node, addSup);
}

//删掉树中第i个结点
void removeNode(fpCondTree *tree, int num)
{
    if(num == tree->n - 1)
	tree->n--;
    else if(num >= 0 && num < tree->n - 1)
    {
	for(int i = num; i < tree->n - 1; i++)
	    tree->nodes[i] = tree->nodes[i + 1];
	tree->n--;
    }
    else
	printf("remove error\n");
}


//从树中任取num个结点的所有组合
void getCombPattern(fpCondTree tree, int x, int minSupport, int begin, int num)//从第begin个元素开始,找num个结点的任意组合
{
    if(num == 0)
    {
	for(int i = pos - 1; i >= 0; i--)
	{
	    minSupport = min(minSupport, result[i].support);
	    printf("| %d ", result[i].name);		
	}
	printf("| %d | : %d\n", x, minSupport);
    }
    else 
	for(int i = begin; i < tree.n - num + 1; i++)
	{
	    result[pos].support = tree.nodes[i].support;
	    result[pos++].name = tree.nodes[i].name;
	    getCombPattern(tree, x, minSupport, i + 1, num - 1);
	    pos--;
	}
}

//fp-growth
//用于挖掘条件fp树(单链,节省空间)中的频繁模式
void fpCondGrowth(fpCondTree condTree, int x)
{
    for(int i = 0; i < condTree.n; i++)//删除未达到阈值的节点
	if(condTree.nodes[i].support < MIN_SUPPORT_NUM)
	{
	    removeNode(&condTree, i);
	    i = -1;
	}

    if(condTree.n > 0)//删除节点后树不为空
    {
	for(int i = 0; i < condTree.n; i++)
	    printf("name %d sup %d i %d\n", condTree.nodes[i].name, condTree.nodes[i].support, i);

	//挖掘此链上所有频繁模式
	for(int i = condTree.n; i > 0; i--)
	{
	    pos = 0;
	    getCombPattern(condTree, x, AFFAIR_NUM, 0, i);		
	}
    }
}

//对整个fp树进行处理,将每个分支分解
void fpGrowth()
{
    //条件fp树,不包含当前结点
    fpCondTree condTree;//条件fp树,记录每一个分支到根节点的路径
    fpNode *tmpNode;//用于遍历同名节点
    fpNode *traceNode;//用于回溯指定节点
    //利用头表遍历fp树,从最后一项开始
    for(int i = headerTabNum - 1; i >= 0; i--)
    {
	condTree.n = 0;
	tmpNode = headerTab[i].p;//初始指向头表链接的第一个节点
	traceNode = tmpNode;
	while(tmpNode != NULL)//还有后续节点
	{
	    //建立当前节点到根节点的路径
	    int addSup = tmpNode->support;//记录当前节点引入的支持度,由此节点向上均按照这个支持度
	    while(getParent(traceNode) != 0)
	    {
		traceNode = &fpTree.nodes[getParent(traceNode)];
		addToCond(&condTree, traceNode, addSup);//把路径上的节点加入条件fp树
	    }
	    tmpNode = tmpNode->p;//取下一个同名节点
	    traceNode = tmpNode;
	}
	//如果获取的路径不为空则调用单链处理进行挖掘
	if(condTree.n > 0)
	    fpCondGrowth(condTree, headerTab[i].name);
    }
}

int main()
{
    parse();
    getHeaderTab();
    initData();
    getFpTree();
    link();
    fpGrowth();
//    printf("MIN SUP %f\n", MIN_SUPPORT_NUM);
    return 0;
}
