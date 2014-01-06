#pragma once

FORCEINLINE
VOID
CheckListEntry(_In_ PLIST_ENTRY Entry)
{
    if ((((Entry->Flink)->Blink) != Entry) ||
        (((Entry->Blink)->Flink) != Entry))
    {
        RaiseException(EXCEPTION_ACCESS_VIOLATION, EXCEPTION_NONCONTINUABLE , 0, NULL);
    }
}

FORCEINLINE
VOID
InitializeListHead(__in PLIST_ENTRY ListHead)
{
    ListHead->Flink = ListHead->Blink = ListHead;
}

FORCEINLINE
VOID
InsertHeadList(__in PLIST_ENTRY ListHead,
               __in PLIST_ENTRY Entry)
{
    PLIST_ENTRY OldFlink;

    OldFlink = ListHead->Flink;
    Entry->Flink = OldFlink;
    Entry->Blink = ListHead;
    OldFlink->Blink = Entry;
    ListHead->Flink = Entry;

    CheckListEntry(Entry);
}

FORCEINLINE
VOID
InsertTailList(__in PLIST_ENTRY ListHead,
               __in PLIST_ENTRY Entry)
{
    PLIST_ENTRY OldBlink;

    OldBlink = ListHead->Blink;
    Entry->Flink = ListHead;
    Entry->Blink = OldBlink;
    OldBlink->Flink = Entry;
    ListHead->Blink = Entry;

    CheckListEntry(Entry);
}

FORCEINLINE
BOOL
IsListEmpty(__in const LIST_ENTRY * ListHead)
{
    return (ListHead->Flink == ListHead);
}

FORCEINLINE
BOOL
RemoveEntryList(__in PLIST_ENTRY Entry)
{
    PLIST_ENTRY OldFlink;
    PLIST_ENTRY OldBlink;

    OldFlink = Entry->Flink;
    OldBlink = Entry->Blink;
    OldFlink->Blink = OldBlink;
    OldBlink->Flink = OldFlink;
    return (BOOL)(OldFlink == OldBlink);
}

FORCEINLINE
PLIST_ENTRY
RemoveHeadList(__in PLIST_ENTRY ListHead)
{
    PLIST_ENTRY Flink;
    PLIST_ENTRY Entry;

    Entry = ListHead->Flink;
    Flink = Entry->Flink;
    ListHead->Flink = Flink;
    Flink->Blink = ListHead;
    return Entry;
}

FORCEINLINE
PLIST_ENTRY
RemoveTailList(__in PLIST_ENTRY ListHead)
{
    PLIST_ENTRY Blink;
    PLIST_ENTRY Entry;

    Entry = ListHead->Blink;
    Blink = Entry->Blink;
    ListHead->Blink = Blink;
    Blink->Flink = ListHead;
    return Entry;
}

FORCEINLINE
ULONG
GetListCount(__in PLIST_ENTRY ListHead)
{
    PLIST_ENTRY CurrentEntry;
    ULONG Count = 0;

    CurrentEntry = ListHead->Flink;
    while (CurrentEntry != ListHead)
    {
        CurrentEntry = CurrentEntry->Flink;
        Count++;
    }
    return Count;
}