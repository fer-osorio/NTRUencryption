// Generates an array were the element i is equal to i%3. Requited for some optimizations.

#include<stdio.h>
#include<stdlib.h>

#define DECIMAL_BASE  10
#define TABLE_WIDTH   75                                                         // Number of columns that the table (representation of the array) will have
#define _q_ 16384

int uintToString(unsigned,char*);                                               // From unsigned integer to string
int copyString(const char* origin,char* dest);                                  // Copies 'origin' on 'dest'
int createqMod3File(unsigned  q);                                               // Creates a file with the int array qmod3, where qmod3[i] = i%3 for all 0 < i < q.

int main(int argc, char* argv[]) {
    createqMod3File(_q_);
    return 0;
}

int uintToString(unsigned n,char* dest) {
    unsigned i = 0, j = 0;
    char buff = 0;
    do {
        buff = (char)(n % DECIMAL_BASE);                                         // Taking last current digit
        dest[i++] = buff + 48;                                                   // Saving last current digit
        n -= (unsigned)buff; n /= DECIMAL_BASE;                                  // Taking out last current digit from the number n
    } while(n > 0);
    dest[i--] = 0;                                                              // Putting a zero at the end and returning one place
    for(; j < i; j++,i--) {                                                     // The number is backwards; reversing the order of the digits
        buff = dest[j];
        dest[j] = dest[i];
        dest[i] = buff;
    }
    return 0;
}

int copyString(const char* origin,char* dest) {
    int i = 0;                                                                  // Counting variable. At the end it will contain the length of the origin string
    for(; origin[i] != 0; i++) {dest[i] = origin[i];}                           // Coping element by element
    dest[i] = 0;                                                                // End of string.
    return i;                                                                   // Returning string length
}

//int qmod3 = {
///*0  */  0,1,2,0,1,2,0,1,2,0,1,2,0,1,2,0,1,2,0,1,2,0,1,2,0,1,2,0,1,2,0,1,2,0,1,2,0,1,2,0,1,2,0,1,2,0,1,2,0,1,2,0,1,2,0,1,2,0,1,2,0,1,2,0,1,2,0,1,2,0,1,2,0,1,2,
///*1  */  0,1,2,0,1,2,0,1,2,...                            .};
//              }
int createqMod3File(unsigned q) {
    const char* header = "char qmod3[";                                         // Header for the array
    const char* closing = "] = {\n";                                            // Closing brackets for the declaration of the array
    const char* firstLeftTag = "/*0  */  ";                                      // First left tag, it will tell us the number of the line
    char qStr[6];                                                               // String representation of q
    char aux[5];                                                                // Auxiliary array
    char* fileCont;                                                              // File content
    int headerSZ = -1;                                                          // Header size
    int leftTagsSZ = -1;                                                        // Left tags size
    int spaceforNum = 0;                                                        // Space for the number inside the tag.
    int fileContSZ = 0;                                                          // File content size
    int numofRows = 0;                                                          // Number of rows
    int rowSz = 0;
    int lastRowSZ = 0;                                                          // Last row size
    int i = 0, j = 0, k = 0;                                                    // Counting variables
    FILE* fptr;                                                                 // Pointer to file;

    while(header[++headerSZ] != 0) {}                                           // Getting header size
    while(firstLeftTag[++leftTagsSZ] != 0) {}                                    // Getting left tags size
    spaceforNum = leftTagsSZ - 6;                                               // Tag size minus the length of /**/ and two spaces

    numofRows = q / TABLE_WIDTH;
    lastRowSZ = (q % TABLE_WIDTH)*2;
    rowSz = leftTagsSZ + TABLE_WIDTH * 2;                                       // Taking in account the numbers and the separator ','(line break not included)

    fileContSZ = headerSZ +                                                        // Size of the header
                leftTagsSZ*numofRows +                                          // Size of all the left tags
                numofRows + 1 +                                                 // Accounts for all the jump line characters
                q*2 +                                                           // Elements of the array and its associated commas
                1 + 1 + 1;                                                      // Closing bracket, semicolon and final 0
    if(lastRowSZ > 0) fileContSZ += leftTagsSZ + lastRowSZ*2;                    // In case of having q as a non-multiple of TABLE_WIDTH line
    fileCont = (char*)malloc(fileContSZ);
    k = copyString(header, fileCont);
    uintToString(q, qStr);
    k += copyString(qStr, &fileCont[k]);
    k += copyString(closing, &fileCont[k]);
    for(j = k, i = 0; i < numofRows; i++) {
        fileCont[j++] = '/';
        fileCont[j++] = '*';
        uintToString((unsigned)i, aux);
        k = copyString(aux, &fileCont[j]);
        while(fileCont[j] != 0) j++;
        while(k < spaceforNum) {
            k++; fileCont[j++] = ' ';
        }
        fileCont[j++] = '*';
        fileCont[j++] = '/';
        fileCont[j++] = ' ';
        fileCont[j++] = ' ';
        for(k = leftTagsSZ; k < rowSz; k += 6) {                                // Here we're assuming TABLE_WITH is a multiple of 3
            fileCont[j++] = '0';
            fileCont[j++] = ',';
            fileCont[j++] = '1';
            fileCont[j++] = ',';
            fileCont[j++] = '2';
            fileCont[j++] = ',';
        }
        fileCont[j++] = '\n';
    }
    if(lastRowSZ > 0) {                                                         // In case of having q not a multiple of TABLE_WIDTH
        lastRowSZ += leftTagsSZ;
        fileCont[j++] = '/';
        fileCont[j++] = '*';
        uintToString((unsigned)i, aux);
        k = copyString(aux, &fileCont[j]);
        while(fileCont[j] != 0) j++;
        while(k < spaceforNum) {
            k++; fileCont[j++] = ' ';
        }
        fileCont[j++] = '*';
        fileCont[j++] = '/';
        fileCont[j++] = ' ';
        fileCont[j++] = ' ';
        for(k = leftTagsSZ; k < lastRowSZ;) {                                   // Here we're assuming TABLE_WITH is a multiple of 3
            fileCont[j++] = '0'; k++;
            fileCont[j++] = ','; k++;
            if(k >= lastRowSZ) break;
            fileCont[j++] = '1'; k++;
            fileCont[j++] = ','; k++;
            if(k >= lastRowSZ) break;
            fileCont[j++] = '2'; k++;
            fileCont[j++] = ','; k++;
        }
        j--;                                                                    // We'll take back the last comma
    }
    fileCont[j++] = '}';
    fileCont[j++] = ';';
    fileCont[j++] = 0;

    fptr = fopen("qmod3.h", "w");
    if(fptr == NULL) {
        printf ("Could not open file, terminating the program...\n");
        return EXIT_FAILURE;
    }
    fprintf(fptr, "%s", fileCont);

    fclose(fptr);
    free(fileCont);
    return 0;
}
