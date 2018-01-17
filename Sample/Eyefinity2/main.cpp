///
///  Copyright (c) 2008 - 2012 Advanced Micro Devices, Inc.
 
///  THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY KIND,
///  EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
///  WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR PURPOSE.

/// \file main.cpp

#include <windows.h>
#include <stdio.h>
#include "eyefinity.h"
#include <string.h>
#include <stdlib.h>

int main (int c,char* k[],char* s[])
{
   
	int iDisplayIndex[6],displayCount=0;
	int temp= 0,iValidParameters=1;
	int iHBezel=0,iVBezel=0;
	int i=0, j=0;
	int validInput=-1;
	int xRes=0,yRes=0;
	char *token, *val, *sSep; 
	char str[256],op;
	int args[3],argc=1;	
	
	memset(args,-1,sizeof(args));
	if (initializeADL())
	{
		sSep = (char*)malloc(sizeof(char));
		if (c== 1)
		{
			printSyntax();
		}
		else
		{
			while (argc++ < c)
			{
				k++;
				strcpy(str, *k);
				*sSep = *str;
				val = strtok (str,sSep);
				if (*sSep == 'i')
				{
					printDisplayIndexes();
				}
				else if (val != NULL)
				{
					switch (*sSep)
					{
					case 'e':				
						token = strtok (val,",");
						while (token != NULL)
						{
							args[temp++] = atoi(token);
							token = strtok (NULL,",");							
						}		
						op ='e';
						break;		
					case 'r':				
						token = strtok (val,",");
						while (token != NULL)
						{
							args[temp++] = atoi(token);
							token = strtok (NULL,",");
						}	
						op ='r';
						break;
					case 'd':				
						token = strtok (val,",");
						while (token != NULL)
						{
							args[temp++] = atoi(token);
							token = strtok (NULL,",");
						}	
						op ='d';
						break;
					case 'b':				
						token = strtok (val,",");
						while (token != NULL)
						{
							args[temp++] = atoi(token);
							token = strtok (NULL,",");
						}	
						op ='b';
						break;
					case 's':				
						token = strtok (val,",");
						while (token != NULL)
						{
							args[temp++] = atoi(token);
							token = strtok (NULL,",");
						}
						op ='s';
						break;
					case 'm':				
						token = strtok (val,",");
						while (token != NULL)
						{
							iDisplayIndex[displayCount++] = atoi(token);
							token = strtok (NULL,",");
						}		
						break;
					case 'x':				
						xRes= atoi(val);			
						break;
					case 'y':				
						yRes= atoi(val);			
						break;	
					case 'v':				
						iVBezel= atoi(val);			
						break;
					case 'h':		
						if ( val!= NULL)
							iHBezel= atoi(val);			
						break;		
					default:
						iValidParameters = 0;
						break;
					}
		
				}
				else
					iValidParameters = 0;
			}

			if (!iValidParameters)
			{
				printf("not a valid parameteres \n");
				printSyntax();
			}
			else if (iHBezel >0 && iVBezel >0 && args[0] != -1 && op =='b')
			{
				setBezelOffsets(args[0], iHBezel, iVBezel);
			}
			else if (xRes >0 && yRes >0 && args[0] != -1 && op =='s')
			{
				setResolution(args[0], xRes, yRes);
			}
			else if (args[0] != -1 && args[1] != -1 && args[2] != -1 && (displayCount == args[1] * args[2]) && op =='e')
			{
				setAdapterDisplaysToEyefinity(args[0], args[1], args[2], iDisplayIndex, displayCount , 0);
			}
			else if (args[0] != -1 && displayCount > 0 && op =='r')
			{
				setAdapterDisplaysToEyefinity(args[0], 0, 0, iDisplayIndex, displayCount , 1);
			}	
			else if (args[0] != -1 && op =='d')
			{
				disableAdapterEyefinityMapping(args[0]);
			}				
		}
		free(sSep);
		deinitializeADL();	
	}
	
    return 0;
}
void printSyntax()
{
	printf ("This Application accepts below parameters\n");
	printf ("-----------------------------------------\n");
	printf ("Display Information \t eyefinity i\n");
	printf ("\t\t \t Ex: eyefinity i\n\n\n");
	printf ("Creat Eyefinity \t eyefinity e<AdapterIndex>,<Columns>,<Rows> \n\t\t\t\t   m<DisplayIndex1,DisplayIndex2...,DisplayIndexN>\n");
	printf ("\t\t \t Ex: eyefinity e13,2,3 m0,2,3,4,5,1\n\n\n");
	printf ("Disable Eyefinity \t eyefinity d<AdapterIndex>\n");
	printf ("\t\t \t Ex: eyefinity d13\n\n\n");
	printf ("Re arrange Eyefinity \t eyefinity r<AdapterIndex> \n\t\t\t\t   m<DisplayIndex1,DisplayIndex2...,DisplayIndexN>\n");
	printf ("\t\t \t Ex: eyefinity r13 m0,2,3,4,5,1\n\n\n");
	printf ("Set Bezel Offsets \t eyefinity b<AdapterIndex> \n\t\t\t\t   h<Horizontal Offset in pixel> \n\t\t\t\t   v<Vertical Offset in pixel>\n");
	printf ("\t\t \t Ex: eyefinity b13 h120 v130\n\n\n");
	printf ("Set Resolution \t\t eyefinity s<AdapterIndex> \n\t\t\t\t   x<Horizontal resolution> \n\t\t\t\t   y<Vertical resolution>\n");
	printf ("\t\t \t Ex: eyefinity s13 x1920 y1200\n\n\n");
	
}

