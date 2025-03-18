#include <stdio.h>
#include <string.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
//#include "program1.asm"
//#include "program2.asm"
//#include "program3.asm"
#include "Instruction.h"
#define MAX_BREAKPOINTS 128
#define VERSION 8
#define MAXLIMIT 2000
#include <stdbool.h>
#include <stddef.h>
int STACK_SIZE = 64;
int HEAP_SIZE = 8192;
int sizeOfObjekts;
typedef struct
{
    void* forward_pointer;
    bool brokenHeart;
    unsigned int size; // # byte of payload
    unsigned char data[1]; // payload data, size as needed!
}* ObjRef;

#include "bigint/build/include//bigint.h"

typedef struct
{
    bool isObjRef;

    union
    {
        ObjRef objRef; // isObjRef = TRUE
        int number; // isObjRef = FALSE
    } u;
} StackSlot;


unsigned int* programMemory;
unsigned int pc = 0;

bool switchedMemory = false;
StackSlot* stack;
unsigned int sp = 0;
unsigned int fp = 0;
unsigned int numberOfInstruction;
unsigned int numberOfStaticVar;
ObjRef rv = NULL; // neue
ObjRef* SDA;
int sdaSize;
int breakpoints[MAX_BREAKPOINTS];
int num_breakpoints = 0;
int debug_mode = 0;
void add_breakpoint(int address);
void remove_breakpoint(int address);
int is_breakpoint(int address);
void* getPrimObjectDataPointer(void* obj);
void initStack(int n);
void debugger();
int countOfPrime;
int countOfComp;
int countDeleted;
int gesamtAddedOnHeap;
int deletedBytes;
// int gesamt;
#define NJBF_MAGIC "NJBF"

typedef struct
{
    char* heap;
    char* activeMemoryStart;
    char* activeMemoryEnd;
    char* freePointer;
} MemoryManagerStruct, *MemoryManager;

MemoryManager memory_manager;
MemoryManager initializeMemory(int n);
void switchMemory();
void printHeap();
ObjRef addObjektOnHeap(int sizeOfObjekt);
void triggerGC();
ObjRef relocate(ObjRef orig);
void copyObjects();
ObjRef copyObjectToFreeMem(ObjRef obj_ref);
void allocateMemory();
void scan();


void execute(int i);
void fatalError(char* msg);
void* popObjRef();

FILE* input;


void pushStackSlot(void*);

int endsWith(const char* str, const char* suffix);

void print_stack();

void printProgram(int opcode);

void printSda();


void run();

void reedData(char* const * argv, int i);
void pushNumber(unsigned int i);
int popNumber();

int main(int argc, char* argv[])
{
    printf("Ninja Virtual Machine started\n");
    //    for (int i = 0; i < argc; i++) {
    //        printf("Argument %d : \"%s\"\n", i, argv[i]);
    //    }
    for (int i = 1; i < argc; i++)
    {
        if (strcmp(argv[i], "--version") == 0)
        {
            printf("Ninja Virtual Machine stopped Version %d.0 (compiled Apr 10 2024, 13:30:00) \n", VERSION);
        }
        else if (strcmp(argv[i], "--help") == 0)
        {
            printf("usage: ./njvm [option] [option] ...\n");
            printf("  --version\t show version and exit\n");
            printf("  --help\t show this help and exit\n");
        }
        else if (strcmp(argv[i], "--debug") == 0)
        {
            debug_mode = 1;
            reedData(argv, i + 1);
        }
        else if (endsWith(argv[i], ".bin"))
        {
            reedData(argv, i);
        }
        else if (strcmp(argv[i], "-- stack") == 0)
        {
            printf( "Give a number to set a STACKSIZE: use (64,128,256) .\n");
            int x;
            scanf("%d", &x);
            STACK_SIZE = x;
        }
        else if (strcmp(argv[i], "--heap") == 0)
        {
            printf( "Give a number to set a HEAPSIZE: use (64,128,256) .\n");
            int x;
            scanf("%d", &x);
            HEAP_SIZE = x;
        }
    }
    if (debug_mode)
    {
        debugger();
    }
    else
    {
        memory_manager = initializeMemory(HEAP_SIZE);

        initStack(STACK_SIZE);
        // printHeap();
        run();
    }
    free(SDA);
    free(stack);
    free(memory_manager->heap);
    free(memory_manager);
    free(programMemory);
    printf("Ninja Virtual Machine stopped\n");
    return 0;
}

//TODO wenn es schwere macht dann löschen


void run()
{
    while (programMemory[pc] != HALT)
    {
        int IR = programMemory[pc];
        pc++;
        execute(IR);
        // print_stack();
    }
    //    printProgram();
    //    printf("%d\n", number);
    //    printf("%c\n", letter);
    // memory_manager = initializeMemory(HEAP_SIZE);
    // printHeap();
    // // switchMemory();
    // // printHeap();
    // // switchMemory();
    // // printHeap();
}

void execute(int i)
{
    int opcode = i >> 24;
    // printProgram(i);
    switch (opcode)
    {
    case HALT:
        exit(0);
    case PUSHC:
        bigFromInt(SIGN_EXTEND(i & 0x00FFFFFF));
        pushStackSlot(bip.res);
        break;
    case ADD:
        bip.op2 = popObjRef();
        bip.op1 = popObjRef();
        bigAdd();
        pushStackSlot(bip.res);
        break;
    case SUB:
        bip.op2 = popObjRef();
        bip.op1 = popObjRef();
        bigSub();
        pushStackSlot(bip.res);
        break;
    case MUL:
        bip.op2 = popObjRef();
        bip.op1 = popObjRef();
        bigMul();
        pushStackSlot(bip.res);
        break;
    case DIV:
        bip.op2 = popObjRef();
        bip.op1 = popObjRef();
        bigDiv();
        pushStackSlot(bip.res);
        break;
    case MOD:
        bip.op2 = popObjRef();
        bip.op1 = popObjRef();
        bigDiv();
        pushStackSlot(bip.rem);
        break;
    case RDINT:
        bigRead(stdin);
        pushStackSlot(bip.res);
        break;
    case WRINT:
        bip.op1 = popObjRef();
        bigPrint(stdout);
        break;
    case RDCHR:{

        //bigRead(stdin);
            char input;
        scanf("%c", &input);
            bigFromInt(input);
        pushStackSlot(bip.res);
        break;
        }
    case WRCHR:
        {
            bip.op1 = popObjRef();
            printf("%c", (char)bigToInt());
            break;
        }

    case PUSHG:
        pushStackSlot(SDA[SIGN_EXTEND(i & 0x00FFFFFF)]);
    // print_stack();
        break;
    case POPG:
        SDA[SIGN_EXTEND(i & 0x00FFFFFF)] = popObjRef();
        break;
    case ASF:
        pushNumber(fp);
        fp = sp;
        sp = sp + SIGN_EXTEND(i & 0x00FFFFFF);
        break;
    case RSF:
        sp = fp;
        fp = popNumber();
        break;
    case PUSHL:
        if (stack[fp + SIGN_EXTEND(i & 0x00FFFFFF)].isObjRef)
        {
            pushStackSlot(stack[fp + SIGN_EXTEND(i & 0x00FFFFFF)].u.objRef);
        }
        else
        {
            pushNumber(stack[fp + SIGN_EXTEND(i & 0x00FFFFFF)].u.number);
        }
        break;
    case POPL:
        stack[fp + SIGN_EXTEND(i & 0x00FFFFFF)] = stack[--sp];
        break;
    case EQ:
        bip.op2 = popObjRef();
        bip.op1 = popObjRef();
        if (bigCmp() == 0)
        {
            bigFromInt(1);
            pushStackSlot(bip.res);
        }
        else
        {
            bigFromInt(0);
            pushStackSlot(bip.res);
        }
        break;
    case NE:
        bip.op2 = popObjRef();
        bip.op1 = popObjRef();
        if (bigCmp() != 0)
        {
            bigFromInt(1);
            pushStackSlot(bip.res);
        }
        else
        {
            bigFromInt(0);
            pushStackSlot(bip.res);
        }
        break;
    case LT:
        bip.op2 = popObjRef();
        bip.op1 = popObjRef();
        if (bigCmp() < 0)
        {
            bigFromInt(1);

            pushStackSlot(bip.res);
        }
        else
        {
            bigFromInt(0);
            pushStackSlot(bip.res);
        }
        break;

    case LE:
        bip.op2 = popObjRef();
        bip.op1 = popObjRef();
        if (bigCmp() <= 0)
        {
            bigFromInt(1);
            pushStackSlot(bip.res);
        }
        else
        {
            bigFromInt(0);
            pushStackSlot(bip.res);
        }
        break;
    case GT:
        bip.op2 = popObjRef();
        bip.op1 = popObjRef();
        if (bigCmp() > 0)
        {
            bigFromInt(1);
            pushStackSlot(bip.res);
        }
        else
        {
            bigFromInt(0);
            pushStackSlot(bip.res);
        }
        break;
    case GE:
        bip.op2 = popObjRef();
        bip.op1 = popObjRef();
        if (bigCmp() >= 0)
        {
            bigFromInt(1);
            pushStackSlot(bip.res);
        }
        else
        {
            bigFromInt(0);
            pushStackSlot(bip.res);
        }
        break;
    case JMP:
        pc = SIGN_EXTEND(i & 0x00FFFFFF);
        break;
    case BRF:
        bip.op1 = popObjRef();
        if (bigToInt() == 0)
        {
            pc = SIGN_EXTEND(i & 0x00FFFFFF);
        }
        break;
    case BRT:
        bip.op1 = popObjRef();
        if (bigToInt() != 0)
        {
            pc = SIGN_EXTEND(i & 0x00FFFFFF);
        }
        break;
    case CALL:
        pushNumber(pc);
        pc = SIGN_EXTEND(i & 0x00FFFFFF);
        break;
    case RET:
        pc = popNumber();
        break;
    case DROP:
        sp -= SIGN_EXTEND(i & 0x00FFFFFF);;
        break;
    case PUSHR:
        pushStackSlot(rv);
        break;
    case POPR:
        rv = popObjRef();
        break;
    case DUP:
        pushStackSlot(stack[sp - 1].u.objRef);

        break;
    case NEW:
        {
            ObjRef record;
            unsigned int objSize;
            int numElements = SIGN_EXTEND(i & 0x00FFFFFF);
            objSize = sizeof(*record) + (numElements * sizeof(void *));
            if ((record = addObjektOnHeap(objSize)) == NULL)
            {
                perror("Malloc: i can´t creat a record. See NEW instruction\n");
            }
            for (int i = 0; i < numElements; i++) {
                GET_REFS_PTR(record)[i] = NULL;
                // countOfComp++;
            }
            record->size = MSB + numElements; // like MSB + n
            pushStackSlot(record);
            // countOfComp++;
            break;
        }
    case GETF:
        {
            ObjRef result = popObjRef();
            result = GET_REFS_PTR(result)[SIGN_EXTEND(i & 0x00FFFFFF)];
            pushStackSlot(result);
            break;
        }
    case PUTF:
        {
            ObjRef value = popObjRef();
            ObjRef objekt = popObjRef();
            GET_REFS_PTR(objekt)[SIGN_EXTEND(i & 0x00FFFFFF)] = value;
            break;
        }
    case NEWA:
        {
            bip.op1 = popObjRef();
            ObjRef array;
            unsigned int objSize;
            int numElements = bigToInt();
            objSize = sizeof(*array) + (numElements * sizeof(void *));
            if ((array = addObjektOnHeap(objSize)) == NULL)
            {
                perror("Malloc: i can´t creat a record. See NEW instruction\n");
            }
            for (int i = 0; i < numElements; i++) {
                GET_REFS_PTR(array)[i] = NULL;
                countOfComp++;
            }
            array->size = MSB + numElements; // like MSB + n
            pushStackSlot(array);
            // countOfComp++;
            break;
        }
    case GETFA:
        {
            bip.op1 = popObjRef();
            ObjRef result = popObjRef();
            result = GET_REFS_PTR(result)[bigToInt()];
            pushStackSlot(result);
            // printHeap();
            break;
        }
    case PUTFA:
        {
            ObjRef value = popObjRef();
            bip.op1 = popObjRef(); // Index
            ObjRef array = popObjRef();
            GET_REFS_PTR(array)[bigToInt()] = value;
            break;
        }
    case GETSZ:
        {
            sp--;
            if (IS_PRIMITIVE(stack[sp].u.objRef))
            {
                bigFromInt(-1);
                pushStackSlot(bip.res);
            }
            else
            {
                bigFromInt(GET_ELEMENT_COUNT(stack[sp].u.objRef));
                pushStackSlot(bip.res);
            }
            break;
        }
    case PUSHN:
        {
            ObjRef nilObj = NULL;
            pushStackSlot(nilObj);
            break;
        }
    case REFEQ:
        {
            ObjRef ref2 = popObjRef();
            ObjRef ref1 = popObjRef();
            if (ref1 == ref2)
            {
                bigFromInt(1);
                pushStackSlot(bip.res);
            }
            else
            {
                bigFromInt(0);
                pushStackSlot(bip.res);
            }
            break;
        }

    case REFNE:
        {
            ObjRef ref2 = popObjRef();
            ObjRef ref1 = popObjRef();
            if (ref1 != ref2)
            {
                bigFromInt(1);
                pushStackSlot(bip.res);
            }
            else
            {
                bigFromInt(0);
                pushStackSlot(bip.res);
            }
            break;
        }
    }
}

ObjRef copyObjectToFreeMem(ObjRef objRef) {
    int sizOfObjekt;

    // Berechne die Größe des Objekts
    if (IS_PRIMITIVE(objRef)) {
        sizOfObjekt = sizeof(*objRef) + objRef->size;
    } else {
        sizOfObjekt = sizeof(*objRef) + GET_ELEMENT_COUNT(objRef) * sizeof(ObjRef);
    }

    // Überprüfe, ob genug freier Speicher vorhanden ist
    if (memory_manager->freePointer + sizOfObjekt > memory_manager->activeMemoryEnd) {
        printf("Memory allocation error after garbage collection.\n");
        exit(1);
    }

    // Kopiere das Objekt in den freien Speicherbereich
    ObjRef copy = (ObjRef)memory_manager->freePointer;
    memcpy(copy, objRef, sizOfObjekt);

    // Setze das Forwarding
    objRef->brokenHeart = true;
    objRef->forward_pointer = copy;
    countDeleted++;
    deletedBytes += sizOfObjekt;
    // Aktualisiere den freien Speicherpointer
    memory_manager->freePointer += sizOfObjekt;

    return copy;
}


// TODO THIS is pseoudo must be implementes
ObjRef relocate(ObjRef orig) {
    ObjRef copy;
    if (orig == NULL) {
        return NULL;
    } else {
        if (orig->brokenHeart) {
            // printf(">>>>>>>>>>>>>>>>>>>>\n");
            return orig->forward_pointer;
        } else {
            copy = copyObjectToFreeMem(orig);
            orig->brokenHeart = true;
            orig->forward_pointer = copy;
            return copy;
        }
    }
}
void scan() {
    char* scan = memory_manager->activeMemoryStart;
    size_t moveScannPointerBytes;

    while (scan < memory_manager->freePointer) {
        ObjRef objekt = (ObjRef)scan;

        if (IS_PRIMITIVE(objekt)) {
            moveScannPointerBytes = sizeof(*objekt) + objekt->size;
        } else {
            moveScannPointerBytes = sizeof(*objekt) + GET_ELEMENT_COUNT(objekt) * sizeof(ObjRef);
            for (int i = 0; i < GET_ELEMENT_COUNT(objekt); ++i) {
                objekt->forward_pointer =relocate(objekt->forward_pointer);
                GET_REFS_PTR(objekt)[i] = relocate(GET_REFS_PTR(objekt)[i]);
            }
        }
        scan += moveScannPointerBytes;
    }
}

void initStack(int n) {
    stack = malloc(n * 1024);
    if (stack == NULL) {
        fprintf(stderr, "Memory allocation failed.\n");
        exit(EXIT_FAILURE);
    }
}
MemoryManager initializeMemory(int n) {
    if ((memory_manager = malloc(sizeof(*memory_manager))) == NULL) {
        perror("malloc i can't create Heap Manager\n");
        exit(1);
    }
    memory_manager->heap = malloc(n * 1024);
    if (memory_manager->heap == NULL) {
        perror("malloc i can't create Heap\n");
        free(memory_manager);
        exit(1);
    }

    memory_manager->activeMemoryStart = memory_manager->heap;
    memory_manager->freePointer = memory_manager->heap;
    memory_manager->activeMemoryEnd = memory_manager->heap + ((n * 1024) / 2);
    return memory_manager;
}


void allocateMemory() {
    rv = relocate(rv);
    bip.op1 = relocate(bip.op1);
    bip.op2 = relocate(bip.op2);
    bip.res = relocate(bip.res);
    bip.rem = relocate(bip.rem);

    for (int i = 0; i < numberOfStaticVar; ++i) {
        SDA[i] = relocate(SDA[i]);
    }

    for (int i = 0; i < sp; ++i) {
        if (stack[i].isObjRef) {
            stack[i].u.objRef = relocate(stack[i].u.objRef);
        }
    }
}
// Funktion zum Auslösen des Garbage Collectors
void triggerGC()
{
    deletedBytes = 0;
    // gesamt = (countOfComp + countOfPrime) - countDeleted;
    //printf("-------  delete: %d   Bytes: %d GHeap: %d DeletedBytes: %d \n", countDeleted,  sizeOfObjekts -deletedBytes,gesamtAddedOnHeap, deletedBytes);
    //printf("Bevor GC\n");
    //printf("Active Memory Start: %p\n", memory_manager->activeMemoryStart);
   // printf("Free Pointer: %p\n", memory_manager->freePointer);
   // printf("Active Memory End: %p\n", memory_manager->activeMemoryEnd);
    // printHeap();

    switchMemory();
    // printf(">>>>>>>>>>>>>>>> Switched >>>>>>>>>>>>>>>>>>>>>>>>>>>>>_-----------------\n");
    // printf(">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>_-----------------\n");
    // printf(">>>>>>>>>>>>>>>> Allocte >>>>>>>>>>>>>>>>>>>>>>>>>>>>>_-----------------\n");
    allocateMemory();
    // printf(">>>>>>>>>>>>>>>> Alloctedd >>>>>>>>>>>>>>>>>>>>>>>>>>>>>_-----------------\n");
    // printf(">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>_-----------------\n");
    // printf(">>>>>>>>>>>>>>>> scan >>>>>>>>>>>>>>>>>>>>>>>>>>>>>_-----------------\n");
    scan();

    //printf("After GC\n");
   // printf("Active Memory Start: %p\n", memory_manager->activeMemoryStart);
   // printf("Free Pointer: %p\n", memory_manager->freePointer);
   // printf("Active Memory End: %p\n", memory_manager->activeMemoryEnd);
    // gesamt = (countOfComp + countOfPrime) - countDeleted;

   // printf("-------  delete: %d   Bytes: %d GHeap: %d DeletedBytes: %d \n", countDeleted,  sizeOfObjekts - deletedBytes,gesamtAddedOnHeap, deletedBytes);
  //  printf(">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>_-----------------\n");

    // printHeap();

    // Implementierung hinzufügen
}

void switchMemory() {
    if (!switchedMemory) {
        memory_manager->activeMemoryStart = memory_manager->activeMemoryEnd;
        memory_manager->freePointer = memory_manager->activeMemoryStart;
        memory_manager->activeMemoryEnd = memory_manager->activeMemoryStart + ((HEAP_SIZE * 1024) / 2);
        switchedMemory = true;
    } else {
        memory_manager->activeMemoryStart = memory_manager->heap;
        memory_manager->freePointer = memory_manager->heap;
        memory_manager->activeMemoryEnd = memory_manager->heap + ((HEAP_SIZE * 1024) / 2);
        switchedMemory = false;
    }
}


void printHeap() {
    printf("Heap content:\n");
    char* current = memory_manager->heap;
    while (current < memory_manager->heap + HEAP_SIZE * 1024) {
        if (current == memory_manager->activeMemoryStart) {
            printf("S");
        } else if (current == memory_manager->freePointer) {
            printf("F");
        } else if (current >= memory_manager->activeMemoryStart && current < memory_manager->freePointer) {
            ObjRef objRef = (ObjRef)current;
            int sizeOfObjekt;
            if (IS_PRIMITIVE(objRef)) {
                sizeOfObjekt = sizeof(* objRef) + objRef->size;
                printf("P");
            } else {
                sizeOfObjekt = sizeof(* objRef) + GET_ELEMENT_COUNT(objRef) * sizeof(* objRef);
                printf("V");
            }
            current += sizeOfObjekt;
            continue;
        } else if (current >= memory_manager->activeMemoryEnd) {
            printf("*");
        } else {
            printf(".");
        }
        current++;
    }
    printf("\n");
}


// Funktion zum addieren eines Objektes
ObjRef addObjektOnHeap(int sizeOfObjekt)
{
    // printf(">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>\n");
    if (memory_manager->freePointer + sizeOfObjekt >= memory_manager->activeMemoryEnd)
    {
        // printHeap();
        triggerGC();

        if (memory_manager->freePointer + sizeOfObjekt >= memory_manager->activeMemoryEnd)
        {
            // printHeap();
            perror("GC can´t give you Memory: Just god can give you free Memory :)\n");
            exit(1);
        }
    }
    gesamtAddedOnHeap++;
    sizeOfObjekts += sizeOfObjekt;
    ObjRef objRef = (ObjRef)memory_manager->freePointer;
    objRef->forward_pointer = NULL;
    objRef->brokenHeart = false;
    memory_manager->freePointer += sizeOfObjekt;
    return objRef;
}


void* getPrimObjectDataPointer(void* obj)
{
    ObjRef oo = ((ObjRef)(obj));
    return oo->data;
}

void fatalError(char* msg)
{

    print_stack();
    // printHeap();
    printf("Fatal Error: %s\n", msg);
    exit(1);
}

void* newPrimObject(int dataSize)
{
    ObjRef objref;
    if ((objref = addObjektOnHeap(sizeof (*(objref)) + dataSize)) == NULL)
    // if ((objref = (ObjRef)malloc(sizeof (*(objref)) + dataSize)) == NULL)
    {
        perror("malloc");
    }

    objref->size = dataSize;
    // printHeap();
    // countOfPrime++;
    return objref;
}

void pushStackSlot(void* i)
{
    ////    ObjRef resultRef;
    StackSlot resultStackSlot;
    //    unsigned int objSize = sizeof(i) + sizeof(int);
    //    if (sp >= MAXLIMIT) {
    //        printf("Stack overflow\n");
    //        exit(1);
    //    }
    //    if ((resultRef = (ObjRef)malloc(objSize)) == NULL) {
    //        perror("malloc");
    //    }
    //    resultRef->size = sizeof(int);
    //    *(int *)resultRef->data = i;
    resultStackSlot.isObjRef = true;
    resultStackSlot.u.objRef = i;
    //    stack[sp++] = resultStackSlot;
    ////    printf(">>>>>>>>>>>>>> %s\n", stack[sp- 1].isObjRef ? "true" : "false");
    stack[sp++] = resultStackSlot;
}

void pushNumber(unsigned int i)
{
    StackSlot resultStackSlot;
    if (sp >= MAXLIMIT)
    {
        printf("Stack overflow\n");
        exit(1);
    }
    resultStackSlot.isObjRef = false;
    resultStackSlot.u.number = i;
    stack[sp++] = resultStackSlot;
}

void* popObjRef()
{
    if (sp <= 0)
    {
        printf("Stack underflow\n");
        exit(1);
    }
    if (stack[sp - 1].isObjRef)
    {
        return stack[--sp].u.objRef;
    }
    else
    {
        printf("Type Error: trying to pop number with objekt Method try: popNumber \n");
        exit(1);
    }
}

int popNumber()
{
    if (sp <= 0)
    {
        printf("Stack underflow\n");
        exit(1);
    }
    if (!stack[sp - 1].isObjRef)
    {
        return stack[--sp].u.number;
    }
    else
    {
        printf("Type Error: trying to pop Objekt with number Method try: popObjRef \n");
        exit(1);
    }
}

void debugger()
{
    char command[256];
    run();
    while (1)
    {
        printf("debugger> ");
        if (fgets(command, sizeof(command), stdin) == NULL)
        {
            printf("\n");
            break;
        }
        if (strncmp(command, "stack", 5) == 0)
        {
            print_stack();
        }
        else if (strncmp(command, "sda", 3) == 0)
        {
            printSda();
        }
        else if (strncmp(command, "list", 4) == 0)
        {
            // printProgram();
        }
        else if (strncmp(command, "step", 4) == 0)
        {
            if (pc < numberOfInstruction)
            {
                // printProgram();
            }
            else
            {
                printf("End of program.\n");
            }
        }
        else if (strncmp(command, "cont", 4) == 0)
        {
            while (pc < numberOfInstruction)
            {
                if (is_breakpoint(pc))
                {
                    printf("Breakpoint at %d\n", pc);
                    break;
                }
                execute(programMemory[pc]);
                pc++;
            }
        }
        else if (strncmp(command, "quit", 4) == 0)
        {
            exit(0);
        }
        else if (strncmp(command, "break", 5) == 0)
        {
            int address = atoi(command + 6);
            add_breakpoint(address);
        }
        else if (strncmp(command, "delete", 6) == 0)
        {
            int address = atoi(command + 7);
            remove_breakpoint(address);
        }
        else if (strncmp(command, "breakpoints", 11) == 0)
        {
            printf("Breakpoints:\n");
            for (int i = 0; i < num_breakpoints; i++)
            {
                printf("%d\n", breakpoints[i]);
            }
        }
        else
        {
            printf("Unknown command: %s", command);
        }
    }
}

void add_breakpoint(int address)
{
    if (num_breakpoints < MAX_BREAKPOINTS)
    {
        breakpoints[num_breakpoints++] = address;
        printf("Breakpoint set at %d\n", address);
    }
    else
    {
        printf("Maximum number of breakpoints reached.\n");
    }
}

void remove_breakpoint(int address)
{
    int found = 0;
    for (int i = 0; i < num_breakpoints; i++)
    {
        if (breakpoints[i] == address)
        {
            found = 1;
            for (int j = i; j < num_breakpoints - 1; j++)
            {
                breakpoints[j] = breakpoints[j + 1];
            }
            num_breakpoints--;
            break;
        }
    }
    if (found)
    {
        printf("Breakpoint removed from %d\n", address);
    }
    else
    {
        printf("No breakpoint found at %d\n", address);
    }
}

int is_breakpoint(int address)
{
    for (int i = 0; i < num_breakpoints; i++)
    {
        if (breakpoints[i] == address)
        {
            return 1;
        }
    }
    return 0;
}

int endsWith(const char* str, const char* suffix)
{
    size_t str_len = strlen(str);
    size_t suffix_len = strlen(suffix);
    const char* start = str + str_len - suffix_len;
    return strcmp(start, suffix) == 0;
}

void reedData(char* const * argv, int pos)
{
    input = fopen(argv[pos], "rb");
    if (!input)
    {
        perror("Error opening input file\n");
        exit(1);
    }
    int readByte = 0;
    int IR;
    int i = 1;
    int it = 0;
    while ((readByte = fread(&IR, sizeof(unsigned int), 1, input) != 0))
    {
        if (i == 1)
        {
            if (IR != 0x46424a4e)
            {
                perror("Error opening input file: Should be NJBF format\n");
                fclose(input);
                exit(1);
            }
            i++;
        }
        else if (i == 2)
        {
            if (IR != VERSION)
            {
                printf("Error opening input file: Unsupported version: %d\n", IR);
                fclose(input);
                exit(1);
            }
            i++;
        }
        else if (i == 3)
        {
            numberOfInstruction = IR;
            if ((programMemory = malloc(numberOfInstruction * sizeof(int))) == NULL)
            {
                perror(" Error: Malloc\n");
                fclose(input);
                exit(1);
            }
            i++;
        }
        else if (i == 4)
        {
            numberOfStaticVar = IR;
            sdaSize = numberOfStaticVar * sizeof(ObjRef);
            if ((SDA = malloc(sdaSize)) == NULL)
            {
                perror("Error: Malloc\n");
                fclose(input);
                exit(1);
            }
            i++;
        }
        else
        {
            programMemory[it] = IR;
            it++;
            // printf("read %d object [%ld bytes]: x = [0x%08x]\n",
            // readByte, readByte * sizeof(unsigned int), IR);
            // printf("%d\n", numberOfInstruction);
        }
    }
}

void printSda()
{
    printf("\n      SDA\n");
    printf(",..........+.......,\n");
    for (int i = numberOfStaticVar - 1; i >= 0; i--)
    {
        if (SDA[i] != 0)
        {
            if (i == sp)
            {
                printf("|sp->%3d|   %5d   |\n", i, *(int*)SDA[i]->data);
            }
            else
            {
                printf("|%7d |   %5d   |\n", i, *(int*)SDA[i]->data);
            }
        }
    }
    printf(",..........+.......,\n\n");
}

void print_stack()
{
    printf("\n      stack\n");
    printf(",..........+.......,\n");
    for (int i = sp; i >= 0; i--)
    {
        if (stack[i].isObjRef)
        {
//            printf("  [%d] ObjRef at %d", i, (int)(stack[i].u.objRef->data));
        }
        else
        {
            printf("  [%d] %d", i, stack[i].u.number);
        }

        // Mark the stack pointer (sp) and frame pointer (fp)
        if (i == sp && i == fp)
        {
            printf(" <- sp, fp");
        }
        else if (i == sp)
        {
            printf(" <- sp");
        }
        else if (i == fp)
        {
            printf(" <- fp");
        }

        printf("\n");
    }
    printf(",..........+.......,\n\n");
}

void printProgram(int i)
{
    int opcode = i >> 24;
    printf("%03d:\t", pc);
    switch (opcode)
    {
    case 0: // HALT
        printf("halt\n");
        break;
    case 1: // PUSHC
        printf("pushc\t%d\n", SIGN_EXTEND(i & 0x00FFFFFF));
        break;
    case 2: // ADD
        printf("add\n");
        break;
    case 3: // SUB
        printf("sub\n");
        break;
    case 4: // MUL
        printf("mul\n");
        break;
    case 5: // DIV
        printf("div\n");
        break;
    case 6: // MOD
        printf("mod\n");
        break;
    case 7: // RDINT
        printf("rdint\n");
        break;
    case 8: // WRINT
        printf("wrint\n");
        break;
    case 9: // RDCHR
        printf("rdchr\n");
        break;
    case 10: // WRCHR
        printf("wrchr\n");
        break;
    case 11: // PUSHG
        printf("pushg\t%d\n", SIGN_EXTEND(i & 0x00FFFFFF));
        break;
    case 12: // POPG
        printf("popg\t%d\n", SIGN_EXTEND(i & 0x00FFFFFF));
        break;
    case 13: // ASF
        printf("asf\t%d\n", SIGN_EXTEND(i & 0x00FFFFFF));
        break;
    case 14: // RSF
        printf("rsf\n");
        break;
    case 15: // PUSHL
        printf("pushl\t%d\n", SIGN_EXTEND(i & 0x00FFFFFF));
        break;
    case 16: // POPL
        printf("popl\t%d\n", SIGN_EXTEND(i & 0x00FFFFFF));
        break;
    case 17: // EQ
        printf("eq\n");
        break;
    case 18: // NE
        printf("ne\n");
        break;
    case 19: // LT
        printf("lt\n");
        break;
    case 20: // LE
        printf("le\n");
        break;
    case 21: // GT
        printf("gt\n");
        break;
    case 22: // GE
        printf("ge\n");
        break;
    case 23: // JMP
        printf("jmp\t%d\n", SIGN_EXTEND(i & 0x00FFFFFF));
        break;
    case 24: // BRF
        printf("brf\t%d\n", SIGN_EXTEND(i & 0x00FFFFFF));
        break;
    case 25: // BRT
        printf("brt\t%d\n", SIGN_EXTEND(i & 0x00FFFFFF));
        break;
    case 26: // CALL
        printf("call\t%d\n", SIGN_EXTEND(i & 0x00FFFFFF));
        break;
    case 27: // RET
        printf("ret\n");
        break;
    case 28: // DROP
        printf("drop\t%d\n", SIGN_EXTEND(i & 0x00FFFFFF));
        break;
    case 29: // PUSHR
        printf("pushr\n");
        break;
    case 30: // POPR
        printf("popr\n");
        break;
    case 31: // DUP
        printf("dup\n");
        break;
    case 32: // NEW
        printf("new\t%d\n", SIGN_EXTEND(i & 0x00FFFFFF));
        break;
    case 33: // GETF
        printf("getf\t%d\n", SIGN_EXTEND(i & 0x00FFFFFF));
        break;
    case 34: // PUTF
        printf("putf\t%d\n", SIGN_EXTEND(i & 0x00FFFFFF));
        break;
    case 35: // NEWA
        printf("newa\n");
        break;
    case 36: // GETFA
        printf("getfa\n");
        break;
    case 37: // PUTFA
        printf("putfa\n");
        break;
    case 38: // GETSZ
        printf("getsz\n");
        break;
    default:
        printf("unknown opcode\n");
        break;
    }
}
