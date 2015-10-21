#include "parse.h"

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
		mailtab[i].data[j] = atoll(tmp2);
	i++;
    }
    free(line);
}

