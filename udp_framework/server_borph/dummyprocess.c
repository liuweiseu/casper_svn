#include <stdio.h>
#include <string.h>

int main(void){
    char stringin[256];
    stringin[0]=NULL;
    while(1){
        printf("last string was: %s\n",stringin);    
        printf("Enter your string: ");
        fgets(stringin,255,stdin);
    }

}
