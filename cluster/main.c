#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <math.h>
#include <string.h>
#include <unistd.h>

#define DATA_FILE "test.data"
#define RESULT_FILE "result.txt"
#define MAX_LINE 1000
#define MAIL_NUM 1000
#define KMEANS_LOOP 10
#define CLUSTER_NUM 2
#define MAX_SAMP_NUM 100
#define MIN_SAMP_NUM 100

typedef struct
{
    float data[57];
    int species;
    char mark;
}mail;

mail mailtab[MAIL_NUM];
mail center[CLUSTER_NUM];
int random[CLUSTER_NUM];
int count[CLUSTER_NUM];
float weight[57];

void parse()
{
    char *line = malloc(sizeof(char) * MAX_LINE);
    char *token, *saveptr, *tmp1, *tmp2;
    FILE *fp;
    fp = fopen(DATA_FILE, "r");
    int i = 0;
    while(i < MAIL_NUM && fgets(line, MAX_LINE, fp) != NULL)
    {
	tmp1 = line;
	tmp2 = token;
	for(int j = 0; j < 57 ; j++, tmp1 = NULL)
	    if((tmp2 = strtok_r(tmp1, ",", &saveptr)) == NULL)
		break;
	    else
		sscanf(tmp2, "%f", &mailtab[i].data[j]);
	i++;
    }
    free(line);
//    close(fp);
}


void sort(float *result)
{
    float swap;
    for(int i = 0; i < MAIL_NUM; i++)
    {
	for(int j = i; j < MAIL_NUM; j++)
	{
	    if(result[i] > result[j])
	    {
		swap = result[i];
		result[i] = result[j];
		result[j] = swap;
	    }
	}
    }
}

void getDimensionArray(int dimension, float *result)
{
    for(int i = 0; i < MAIL_NUM; i++)
    {
	result[i] = mailtab[i].data[dimension];
    }
}

void init()
{
    float max[57],min[57];
    float *result = malloc(sizeof(float) * MAIL_NUM);
    float newCoordinate;

    memset(max, 0, 57 * sizeof(float));
    memset(min, 0, 57 * sizeof(float));

    for(int i = 0; i < 57; i++)
    {
	getDimensionArray(i, result);
	sort(result);

	for(int j = 0; j < MIN_SAMP_NUM; j++)
	    min[i] += result[j];
	min[i] /= MIN_SAMP_NUM;

	for(int j = MAIL_NUM; j > MAIL_NUM - MAX_SAMP_NUM; j--)
	    max[i] += result[j-1];
	max[i] /= MAX_SAMP_NUM;
    }

    for(int i = 0; i < MAIL_NUM; i++ )
	for(int j = 0; j < 57; j++)
	{
	    if((max[j] - min[j]) != 0)
	    {
		newCoordinate = (mailtab[i].data[j] - min[j]) / (max[j] - min[j]);

		if(newCoordinate > 1)
		    mailtab[i].data[j] = 1;
		else if(newCoordinate < 0)
		    mailtab[i].data[j] = 0;
		else
		    mailtab[i].data[j] = newCoordinate;
	    }
	}
    free(result);
}

float weightDist(mail x, mail y)
{
    float sigma = 0;
    for(int i = 0; i < 57; i++)
	sigma += pow(weight[i], 2) * pow(fabs(x.data[i] - y.data[i]), 2);	    
    return sqrt(sigma);
}

float dist(mail x, mail y)
{
    float sigma = 0;
    for(int i = 0; i < 57; i++)
	sigma += pow(fabs(x.data[i] - y.data[i]), 2);	    
    return sqrt(sigma);
}


void randomCenter()
{
    int seed, tmp, j;
    printf("input a seed: ");
    scanf("%d", &seed);
    srand(seed);
    for(int i = 0; i < CLUSTER_NUM; i++)
    {
	tmp =  rand() % MAIL_NUM;
	for(j = 0; j < i; j++)
	{
	    if(random[j] == tmp)
		break;
	}
	if(j >= i)
	    random[i] = tmp;
	else
	    i--;
    }

    for(int i = 0; i < CLUSTER_NUM; i++)
	printf("random %d\n", random[i]);
}

float SSE()
{
    float sum = 0;
    for(int i = 0; i < MAIL_NUM; i++)
	sum += pow(dist(mailtab[i], center[mailtab[i].species]), 2);
    return sum;
}

float sim(mail x, mail y)
{
    float sum = 0;
    float sumX = 0, sumY = 0;
    float powX = 0, powY = 0;
    float meanX = 0, meanY = 0;
    float modX = 0, modY = 0, modXd = 0, modYd = 0;
    
    for(int i = 0; i < 57; i++)
    {
	sumX += x.data[i];
	sumY += y.data[i];
	powX += pow(x.data[i], 2);
	powY += pow(y.data[i], 2);
    }
    meanX = sumX / 57;
    meanY = sumY / 57;
    modX = sqrt(powX);
    modY = sqrt(powY);

    for(int i = 0; i < 57; i++)
    {
	modXd += pow(x.data[i] - meanX, 2);
	modYd += pow(y.data[i] - meanY, 2);
    }
    modXd = sqrt(modXd);
    modYd = sqrt(modYd);
    for(int i = 0; i < 57; i++)
	sum += x.data[i] * y.data[i];
//	sum += (x.data[i] - meanX) * (y.data[i] - meanY);
    return sum;
}


float OS()
{
    mail c[MAIL_NUM][CLUSTER_NUM];
    float sum[CLUSTER_NUM];
    float result = 0;
    int n[CLUSTER_NUM];
    memset(c, 0, sizeof(c));
    memset(sum, 0, CLUSTER_NUM * sizeof(sum));
    memset(n, 0, CLUSTER_NUM * sizeof(int));
    
    for(int j = 0; j < CLUSTER_NUM; j++)
	for(int i = 0; i < MAIL_NUM; i++)
	{
	    if(mailtab[i].species == j)	
		c[n[j]++][j] = mailtab[i];
	}

    for(int spec = 0; spec < CLUSTER_NUM; spec++)
    {
	for(int i = 0; i < n[spec] - 1; i++)
	    for(int j = i + 1; j < n[spec]; j++)
	    {
		sum[spec] += sim(c[i][spec], c[j][spec]);
	    }
	sum[spec] /= pow(n[spec] - 1, 2);
    }


    for(int i = 0; i < CLUSTER_NUM; i++)
	result += ((n[i] -1) / MAIL_NUM) * sum[i];
    return result;
}

void getWeight()
{
    float e[57], sum[57];

    memset(e, 0, 57 * sizeof(float));
    memset(sum, 0, 57 * sizeof(float));
    memset(weight, 0, 57 * sizeof(float));

    for(int i = 0; i < 57; i++)
	for(int j = 0; j < MAIL_NUM; j++)
	    sum[i] += mailtab[j].data[i];

    for(int i = 0; i < 57; i++)
    {
	for(int j = 0; j < MAIL_NUM; j++)
	    if(mailtab[j].data[i] / sum[i] > 0)
		e[i] += (mailtab[j].data[i] / sum[i]) * log(mailtab[j].data[i] / sum[i]);
	e[i] = - 1 / log(MAIL_NUM) * e[i];
    }

    memset(sum, 0, 57 * sizeof(float));
    for(int i = 0; i < 57; i++)
    {
	for(int j = 0; j < 57; j++)
	    sum[j] += 1 - e[j];
	weight[i] = (1 - e[i]) / sum[i];
	printf("Dimension %d weight is: %f\n", i, weight[i]);
    }
}

void kmeans()
{
    randomCenter();
    int clusterMark;
    float minDist;

    for(int i = 0; i < CLUSTER_NUM; i++)
	center[i] = mailtab[random[i]];

    for(int loop = 0; loop < KMEANS_LOOP; loop++)
    {
	memset(count, 0, CLUSTER_NUM * sizeof(int));
	for(int i = 0, j = 0; i < MAIL_NUM; i++)
	{
	    minDist = weightDist(mailtab[i], center[0]);
	    clusterMark = 0;
	    for(j = 0; j < CLUSTER_NUM; j++)
	    {
		if(weightDist(mailtab[i], center[j]) < minDist)
		{
		    minDist = weightDist(mailtab[i], center[j]);
		    clusterMark = j;
		}
	    }
	    mailtab[i].species = clusterMark;
	    count[clusterMark]++;
	}

	printf("+-------------------------+\n");	
	printf("LOOP %d result:\n", loop + 1);
	for(int i = 0; i < CLUSTER_NUM; i++)
	    printf("Cluster %d has %d members\n", i, count[i]);

	for(int i = 0; i < CLUSTER_NUM; i++)
	    memset(center[i].data, 0, 57 * sizeof(float));

	for(int i = 0; i < MAIL_NUM; i++)
	{
	    for(int j = 0; j < 57; j++)
		center[mailtab[i].species].data[j] += mailtab[i].data[j];
	}
	
	for(int i = 0; i < 57; i++)
	{
	    for(int j = 0; j < CLUSTER_NUM; j++)
		if(count[j] > 0)
		    center[j].data[i] /= count[j];
		else
		    break;
	}
	printf("SSE is: %f\n", SSE());
	printf("OS is : %f\n", OS());
    }
}

void output()
{
/*
    int lastSpec = 0;
    int tmp;
    int LOCK = 0;//0=FALSE, 1=TRUE
    for(int n = 0; n < CLUSTER_NUM; n++)
    {
	for(int i = 0; i < MAIL_NUM; i++)
	{
	    if(mailtab[i].species > n)
	    {
		if(LOCK == 0)
		{
		    tmp = mailtab[i].species;
		    LOCK = 1;
		}
		if(mailtab[i].species == tmp)
		    mailtab[i].species = n;
		else if(mailtab[i].species == n)
		    mailtab[i].species = tmp;
	    }
	}
	LOCK = 0;
    }
*/
    char sig = 'a';
    FILE *fp;
    fp = fopen(RESULT_FILE, "wr");
    for(int i = 0; i < MAIL_NUM; i++)
    {
	mailtab[i].mark = sig + mailtab[i].species;
	fprintf(fp, "%c\n", mailtab[i].mark);
    }
//    close(fp);
}


int main()
{
    parse();
    init();
    getWeight();
    kmeans();
    output();
    return 0;
}
