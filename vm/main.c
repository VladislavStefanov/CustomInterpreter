#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>
#include <stdbool.h>

enum Operation
{
    NOP = 0x00,
    SET = 0x95,
    LOAD = 0x5D,
    STORE = 0x63,
    JMP = 0x91,
    SGZ = 0x25,
    ADD = 0xAD,
    MUL = 0x33,
    DIV = 0x04,
    MOD = 0xB5,
    OUT = 0xC1,
    SLEEP = 0xBF
};

struct Instruction
{
    unsigned char operation;
    int64_t operand1;
    int64_t operand2;
    int64_t operand3;
};

bool isBetweenZeroAnd(int64_t address, uint32_t top)
{
    return address < top && address >= 0;
}

int main(int argc, char *argv[])
{
    if (argc < 1)
    {
        printf("No file supplied!\n");
        return 0;
    }

    char *fileName = argv[1];

    FILE *filePtr;
    filePtr = fopen(fileName, "rb");

    if (filePtr == NULL)
    {
        printf("Couldn't open file!\n");
        return 0;
    }

    char orcIndicator[4];
    fgets(orcIndicator, 4, filePtr);

    if (feof(filePtr))
    {
        fclose(filePtr);
        printf("Use right format!\n");
        return 0;
    }

    if (strcmp("ORC", orcIndicator) != 0)
    {
        fclose(filePtr);
        printf("Use right format!\n");
        return 0;
    }

    uint32_t memorySize = 0;
    fread(&memorySize, sizeof(memorySize), 1, filePtr);
    if (memorySize <= 0)
    {
        fclose(filePtr);
        return 0;
    }

    int64_t *memory = malloc(memorySize * sizeof(int64_t));

    int instructionsSize = 4;
    struct Instruction *instructions = malloc(sizeof(struct Instruction) * instructionsSize);

    int index = 0;
    while(!feof(filePtr))
    {
        if (index >= instructionsSize)
        {
            instructionsSize *= 2;
            instructions = realloc(instructions, sizeof(*instructions) * instructionsSize);
        }

        const char INVALID_INSTRUCTION_FORMAT[] = "Invalid instructions format!\n";

        fread(&instructions[index].operation, sizeof(instructions[index].operation), 1, filePtr);
        if (feof(filePtr))
        {
            break;
        }
        fread(&instructions[index].operand1, sizeof(instructions[index].operand1), 1, filePtr);
        if (feof(filePtr))
        {
            printf(INVALID_INSTRUCTION_FORMAT);
            return 0;
        }
        fread(&instructions[index].operand2, sizeof(instructions[index].operand2), 1, filePtr);
        if (feof(filePtr))
        {
            printf(INVALID_INSTRUCTION_FORMAT);
            return 0;
        }
        fread(&instructions[index].operand3, sizeof(instructions[index].operand3), 1, filePtr);

        index++;
    }
    instructionsSize = index;

    fclose(filePtr);

    index = 0;
    while (index < instructionsSize)
    {
        int64_t operand1 = instructions[index].operand1;
        int64_t operand2 = instructions[index].operand2;
        int64_t operand3 = instructions[index].operand3;
        const char INVALID_ADDRESS[] = "Invalid address!\n";

        switch(instructions[index].operation)
        {
            case NOP:
                break;
            case SET:
                if (!isBetweenZeroAnd(operand1, memorySize))
                {
                     printf(INVALID_ADDRESS);
                     return 0;
                }
                memory[operand1] = operand2;
                break;
            case LOAD:
                if (!isBetweenZeroAnd(operand1, memorySize) || !isBetweenZeroAnd(operand2, memorySize) || !isBetweenZeroAnd(memory[operand2], memorySize))
                {
                     printf(INVALID_ADDRESS);
                     return 0;
                }
                memory[operand1] = memory[memory[operand2]];
                break;
            case STORE:
                if (!isBetweenZeroAnd(operand1, memorySize) || !isBetweenZeroAnd(operand2, memorySize) || !isBetweenZeroAnd(memory[operand1], memorySize))
                {
                     printf(INVALID_ADDRESS);
                     return 0;
                }
                memory[memory[operand1]] = memory[operand2];
                break;
            case JMP:
                if (!isBetweenZeroAnd(operand1, memorySize))
                {
                    printf(INVALID_ADDRESS);
                    return 0;
                }
                if (!isBetweenZeroAnd(memory[operand1], instructionsSize))
                {
                    printf("Invalid instruction index!\n");
                    return 0;
                }
                index = memory[operand1] - 1;
                break;
            case SGZ:
                if (!isBetweenZeroAnd(operand1, memorySize))
                {
                    printf(INVALID_ADDRESS);
                    return 0;
                }
                if (memory[operand1] > 0)
                {
                    index++;
                }
                break;
            case ADD:
                if (!isBetweenZeroAnd(operand1, memorySize) || !isBetweenZeroAnd(operand2, memorySize) || !isBetweenZeroAnd(operand3, memorySize))
                {
                    printf(INVALID_ADDRESS);
                    return 0;
                }
                memory[operand1] = memory[operand2] + memory[operand3];
                break;
            case MUL:
                if (!isBetweenZeroAnd(operand1, memorySize) || !isBetweenZeroAnd(operand2, memorySize) || !isBetweenZeroAnd(operand3, memorySize))
                {
                    printf(INVALID_ADDRESS);
                    return 0;
                }
                memory[operand1] = memory[operand2] * memory[operand3];
                break;
            case DIV:
                if (!isBetweenZeroAnd(operand1, memorySize) || !isBetweenZeroAnd(operand2, memorySize) || !isBetweenZeroAnd(operand3, memorySize))
                {
                    printf(INVALID_ADDRESS);
                    return 0;
                }
                if (memory[operand2] == 0)
                {
                    printf("Cannot divide by zero!\n");
                    break;
                }
                memory[operand1] = memory[operand2] / memory[operand3];
                break;
            case MOD:
                if (!isBetweenZeroAnd(operand1, memorySize) || !isBetweenZeroAnd(operand2, memorySize) || !isBetweenZeroAnd(operand3, memorySize))
                {
                    printf(INVALID_ADDRESS);
                    return 0;
                }
                if (memory[operand2] == 0)
                {
                    printf("Cannot divide by zero!\n");
                    break;
                }
                memory[operand1] = memory[operand2] % memory[operand3];
                break;
            case OUT:
                if (!isBetweenZeroAnd(operand1, memorySize))
                {
                    printf(INVALID_ADDRESS);
                    return 0;
                }
                printf("%c", (char) memory[operand1]);
                break;
            case SLEEP:
                if (!isBetweenZeroAnd(operand1, memorySize))
                {
                    printf(INVALID_ADDRESS);
                    return 0;
                }
                sleep(memory[operand1] / 1000.0);
                break;
            default:
                printf("Invalid instruction!\n");
        }

        index++;
    }


    free(instructions);
    return 0;
}
