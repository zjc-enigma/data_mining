#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define LINE_LEN 500
#define DEMENSION_NUM 23
#define AFFAIR_NUM 8124
#define DATA_FILE "mushroom.dat"
#define ITEM_NUM 120
#define MAX_ITEM_NUM 10
#define MAX_TREE_SIZE 30000
#define MAX_PATTERN_NUM 10000
#define MIN_SUPPORT_PERCENT 0.05
#define MIN_SUPPORT_NUM MIN_SUPPORT_PERCENT * AFFAIR_NUM
#define TRUE 1
#define FALSE 0

typedef unsigned char bool;

typedef struct fpNode
{
    int demension;//在原始数据中的维度
    int name;
    int support;
    int parent;//指示fpTree中父节点
    struct fpNode *p;//指向同名节点
}fpNode;

typedef struct
{
    fpNode nodes[MAX_TREE_SIZE];
    int r;//根
    int n;//节点数
}fpTree;

typedef struct
{
    int list[ITEM_NUM];
    int n;//数目
    int support;
}pattern;


FILE *fp;
char *tonken, *saveptr, *tmpline, *tmptoken;

int data[AFFAIR_NUM][DEMENSION_NUM];
int patternNum = 0;

fpNode headerTab[ITEM_NUM];
fpNode fpData[AFFAIR_NUM][DEMENSION_NUM];
fpTree tree;
pattern fpPattern[MAX_PATTERN_NUM];

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
	    for(k = 0; k < n; k++)
	    {
		if(headerTab[k].demension == j && headerTab[k].name == data[i][j])//若有记录
		{
		    headerTab[k].support++;
		    break;
		}
	    }
	    if(k >= n)//无记录
	    {
		headerTab[n].name = data[i][j];
		headerTab[n].demension = j;
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
}

//按支持度从高到低重排每一个事务的项集,去除达不到阈值的项
void initData()
{
    for(int i = 0; i < AFFAIR_NUM; i++)
    {
	int j = 0;
	for(int k = 0; k < ITEM_NUM && headerTab[k].support > MIN_SUPPORT_NUM; k++)
	    if(data[i][headerTab[k].demension] == headerTab[k].name) //根据维度对比取值
	    {
		fpData[i][j].name = headerTab[k].name;
		fpData[i][j].demension = headerTab[k].demension;
		j++;
	    }
    }
}

void getFpTree()
{
    tree.n = 1;
    tree.r = 0;
    tree.nodes[0].name = -1;
    tree.nodes[0].parent = -1;
    int currentParent;

    for(int i = 0; i < AFFAIR_NUM; i++)
    {
	currentParent = 0;//每个事务均从根节点开始
	for(int j = 0; j < DEMENSION_NUM && fpData[i][j].name != 0; j++)//name == 0 表明此项已删除
	{
	    int k;
	    //若路径重叠,则遍历找子节点,经过的节点计数加一
	    for(k = 1; k < tree.n; k++)
		if(tree.nodes[k].name == fpData[i][j].name && tree.nodes[k].parent == currentParent)
		{
		    tree.nodes[k].support++;
		    currentParent = k;
		    break;
		}

	    //加一个新节点
	    if(k >= tree.n)
	    {
		tree.nodes[tree.n].name = fpData[i][j].name;
		tree.nodes[tree.n].parent = currentParent;
		tree.nodes[tree.n].support = 1;
		currentParent = tree.n;
		tree.n++;
	    }
	}
    }
//    for(int i = 0; i < tree.n; i++)
//	printf("parent %d name %d support %d\n", tree.nodes[i].parent, tree.nodes[i].name, tree.nodes[i].support);
//    printf("tree num is %d\n", tree.n);
}

//把headerTab,fpTree中同名节点建立链接
void link()
{
    for(int i = 0; i < ITEM_NUM; i++)
	headerTab[i].p = NULL;
    for(int i = 0; i < tree.n; i++)
	tree.nodes[i].p = NULL;

    //headerTab指向fp树中第一个同名节点
    for(int i = 0; i < ITEM_NUM; i++)
	for(int j = 0; j < tree.n; j++)
	    if(headerTab[i].name == tree.nodes[j].name)
	    {
		headerTab[i].p = &tree.nodes[j];
		break;
	    }

    //fp树各节点指向最近的同名节点
    for(int i = 0; i < tree.n; i++)
	for(int j = i; j < tree.n; j++)
	    if(tree.nodes[i].name == tree.nodes[j].name)
	    {
		tree.nodes[i].p = &tree.nodes[j];
		break;
	    }
}

int getParent(fpNode *child)
{
    return child->parent;
}

//把b合并至a后,support值不变
void mergePattern(pattern *a, pattern *b)
{
    for(int i = 0; i < b->n; i++)
    {
	a->list[a->n + i] = b->list[i];
    }
    a->n += b->n;
}

void insert(fpTree *condTree, fpNode *insertNode)
{
    condTree->n++;
    condTree->nodes[condTree->n - 1] = *insertNode;
    if(insertNode->parent != 0)
	condTree->nodes[condTree->n - 1].parent = condTree->n;
}

//删掉树中第i个结点
void removeNode(fpTree *Tree, int num)
{
    if(num == Tree->n - 1)
	Tree->n--;
    else if(num > 0 && num < Tree->n - 1)
	for(int i = num; i < Tree->n; i++)
	{
	    Tree->nodes[i] = Tree->nodes[i + 1];
	    Tree->n--;
	}
}

//从树中任取num个结点的所有组合
void getCombPattern(fpTree Tree, pattern x)
{
    pattern result;
    pattern tmp;

    if(Tree.n == 2)
    {
	result.list[0] = Tree.nodes[1].name;
	result.n = 1;
	result.support = Tree.nodes[1].support;

	if(result.support < x.support)
	    mergePattern(&result, &x);
	else 
	{
	    mergePattern(&result, &x);
	    result.support = x.support;
	}
	for(int i = 0; i < result.n; i++)
	    printf("%d ", result.list[i]);
	printf("support : %d\n", result.support);
    }
    else if(Tree.n > 2)
    {
	tmp.list[0] = Tree.nodes[Tree.n - 1].name;
	tmp.n = 1;
	tmp.support = Tree.nodes[Tree.n - 1].support;

	if(Tree.nodes[Tree.n - 1].support < x.support)
	    mergePattern(&tmp, &x);
	else
	{
	    mergePattern(&tmp, &x);
	    tmp.support = x.support;
	}
	printf("%d ", Tree.nodes[Tree.n - 1].name);
	removeNode(&Tree, Tree.n - 1);
	getCombPattern(Tree, tmp);
    }
}
/*
void output(fpTree T, pattern x)
{

    //求取一个至取n个的所有可能
    for(int i = 1; i < T.n; i++)
    {

	getCombPattern(T, i, result);
	if(result.support < x.support)
	    mergePattern(&result, &x);
	else 
	{
	    mergePattern(&result, &x);
	    result.support = addition.support;
	}
	//打印结果
	for(int j = 0; j < result.n; j++)
	{
	    printf("| %d ", result.list[j]);
	}
	printf("| supporting: %d\n", result.support);
    }

}
*/
/*
  int i;
  pattern tmp;
  for(i = 1; i < Tree.n; i++)
  {
  tmp.list[0] = Tree.nodes[i].name;
  tmp.n = 1;
  tmp.support = Tree.nodes[i].support;


  if(Tree.nodes[i].support < addition.support)
  mergePattern(&tmp, &addition);
  else
  {
  mergePattern(&tmp, &addition);
  tmp.support = addition.support;
  }
  }
  if(num == 1)
  {
  for(int j = 0; j < tmp.n; j++)
  {
  printf("| %d ", tmp.list[j]);
  }
  printf("| supporting: %d\n", tmp.support);
  }

  else if(num > 1)
  {
  removeNode(&Tree, i);
  getCombPattern(Tree, num - 1, addition);
  }
  }
*/

//fp-growth
void fpGrowth(fpTree T, pattern x, bool isSingle)
{
//    bool isSingle = TRUE;
    //有唯一子结点时为单路径
//    for(int i = 1; i < T.n - 1; i++)
//	for(int j = i; j < T.n; j++)
//	    if(T.nodes[i].parent == T.nodes[j].parent)
//	    {
//		isSingle = FALSE;
//		break;
//	    }
    //单路径
    if(isSingle)
    {
	if(x.support >= MIN_SUPPORT_NUM)//只有x支持度达到阈值才有必要继续
	{
	    for(int i = 1; i < T.n; i++)//删除未达到阈值的节点
		if(T.nodes[i].support < MIN_SUPPORT_NUM)
		    removeNode(&T, i);
	    getCombPattern(T, x);
	}
    }
    //多路径
    else
    {
	pattern tmpPattern;
	//条件fp树,不包含当前结点
	fpTree condTree;
	fpNode *tmpNode;

	for(int i = 1; i < T.n; i++)
	{
	    tmpPattern.list[0] = T.nodes[i].name;
	    tmpPattern.n = 1;
	    tmpPattern.support = T.nodes[i].support;
	    mergePattern(&tmpPattern, &x);

	    condTree.n = 1;
	    condTree.r = 0;
	    condTree.nodes[0].name = -1;
	    condTree.nodes[0].parent = -1;

	    tmpNode = &T.nodes[i];

	    while(getParent(tmpNode) != 0)
	    {
		tmpNode = &T.nodes[getParent(tmpNode)];
		insert(&condTree, tmpNode);			
	    }
//	    for(int i = 0; i < condTree.n; i++)
//		printf("name %d support %d parent %d i %d\n", condTree.nodes[i].name, condTree.nodes[i].support, condTree.nodes[i].parent, i);
	    if(condTree.n > 1)
		fpGrowth(condTree, tmpPattern, TRUE);
	}
    }
}

int main()
{
    pattern blank;
    memset(blank.list, 0, sizeof(blank.list));
    blank.n = 0;
    blank.support = 0;

    parse();
    getHeaderTab();
    initData();
    getFpTree();
    link();
    fpGrowth(tree, blank, FALSE);
    return 0;
}
