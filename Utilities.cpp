//
// Created by Aidan Milligan on 2019-09-14.
//
#include "Utilities.h"
#include <iostream>

char** Utilities::SplitByCharacter(char * str, char splitChar)
{
    int stringSize = strlen(str),
            splitSize = 0;
    for(int i=0; i<stringSize-1; i++) {
        if (str[i] == splitChar) {
            splitSize++;
        }
    }

    splitSize += 1;

    char ** store;
    store = (char**)malloc((size_t)(splitSize * sizeof(char*)));
    for(int i =0; i < splitSize; i++)
    {
        store[i] = (char*)malloc(stringSize);
    }

    int splitIndex = 0,
            placementIndex = 0;
    for(int i = 0; i<stringSize; i++)
    {
        if(str[i]==splitChar)
        {
            store[splitIndex][placementIndex] = '\0';

            splitIndex++;
            placementIndex=0;
            continue;
        }
        store[splitIndex][placementIndex] = str[i];
        placementIndex++;
    }
    return store;
}

bool Utilities::isValidIpAddress(char*addr)
{
    int splitCount=0;
    for(int i = 0; i < strlen(addr); i++)
    {
        if(addr[i] ==  '.')
            splitCount++;
    }
    if(splitCount < 3)
        return true;
    return false;
}