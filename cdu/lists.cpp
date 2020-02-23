/*======================================================================
** Linked list code
** Copyright (c) 2005 Don Lafontaine
** ALL RIGHTS RESERVED
** By Don Lafontaine
**----------------------------------------------------------------------
**      These are the routines to handle doubly linked lists.  There
** may be a better way to do this.  
**
** DATE					COMMENTS						WHO
**----------------------------------------------------------------------
** 2005/01				FIrst draft						Don
**====================================================================*/
#include <windows.h>
#include <malloc.h>
#include "cdu_structs.h"
#include "cdu_pages.h"

int init_FlightPlan()
	{
	FlightPlan = (FP *)malloc(sizeof(FP));
	FlightPlan->Next = NULL;
	FlightPlan->Prev = NULL;
	FPFirst = FlightPlan;
	FPLast = FlightPlan;

	ActiveFlightPlan = (FP *)malloc(sizeof(FP));
	ActiveFlightPlan->Next = NULL;
	ActiveFlightPlan->Prev = NULL;
	ActiveFPFirst = ActiveFlightPlan;
	ActiveFPLast = ActiveFlightPlan;

	return(0);
	}

void DelFP(FP *entry)
	{
	if (entry->Prev != NULL)
		{
		if (entry->Next != NULL)
			entry->Prev->Next = entry->Next;
		else
			entry->Prev->Next = NULL;
		}
	else if (entry->Next != NULL)
		FPFirst = entry->Next;

	if (entry->Next != NULL)
		{
		if (entry->Prev != NULL)
			entry->Next->Prev = entry->Prev;
		else
			entry->Next->Prev = NULL;
		}
	else if (entry->Prev != NULL)
		FPLast = entry->Prev;

	free(entry);
	return;
	}

// Insert a flightplan entry <entry> after <position>

void InsFP(FP *position, FP *entry)
	{
	if (position->Next != NULL)
		{
		position->Next->Prev = entry;
		entry->Next = position->Next;
		entry->Prev = position;
		position->Next = entry;
		}
	else
		{
		position->Next = entry;
		entry->Prev = position;
		entry->Next = NULL;
		FPLast = entry;
		}
	}

FP *FindFP(char *st)
	{
	FP *Current = FPFirst;
	while (Current)
		{
		if (!strcmp(st, Current->wptid))
			return(Current);
		Current = Current->Next;
		}
	return(NULL);
	}

int FPCount(void)
	{
	static int count;

	count = 0;
	FP *Current = FPFirst;
	while (Current)
		{
		count++;
		Current = Current->Next;
		}
	return(count);
	}
