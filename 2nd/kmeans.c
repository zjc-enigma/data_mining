#include "kmeans.h"

void kmeans()
{
    int count1,count2;
    mail center1 = mailtab[320];
    mail center2 = mailtab[300];

//    srand()

    for(int loop = 0; loop < KMEANS_LOOP; loop++)
    {
	count1 = 0;
	count2 = 0;
	for(int i = 0; i < MAIL_NUM; i++)
	{
	    if(dist(mailtab[i], center1) < dist(mailtab[i], center2))
	    {
		mailtab[i].species = 0;
		count1++;
	    }
	    else
	    {
		mailtab[i].species = 1;
		count2++;
	    }
//	    printf("dist to cluster1 is %f\n", dist(mailtab[i], center1));
//	    printf("dist to cluster2 is %f\n", dist(mailtab[i], center2));
	}
	printf("species 1 has %d members\n", count1);
	printf("species 2 has %d members\n", count2);

	memset(center1.data, 0, 57 * sizeof(float));
	memset(center2.data, 0, 57 * sizeof(float));

	for(int j = 0; j < MAIL_NUM; j++)
	{
	    if(mailtab[j].species == 0)
		for(int k = 0; k < 57; k++)
		    center1.data[k] += mailtab[j].data[k];
	    else
		for(int k = 0; k < 57; k++)
		    center2.data[k] += mailtab[j].data[k];
	}

	for(int i = 0; i < 57; i++)
	{
	    if(count1 > 0 && count2 > 0)
	    {
		center1.data[i] /= count1;
		center2.data[i] /= count2;
	    }
	    else
	    {
		printf("count error\n");
		break;		    
	    }
	}
    }
}
