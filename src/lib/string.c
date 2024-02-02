#include "../lib-header/string.h"
#include "../lib-header/memory_manager.h"
#include "../lib-header/stdtype.h"

void int_to_string(int x, char str[]){
    int i = 0;
    int negative = 0;

    if(x < 0){
        x = x*(-1);
        negative = 1;
    }

    do{
        str[i] = x % 10 + '0';
        i++; 
    } while  ( (x /= 10) > 0);

    if(negative){
        str[i] = '-';
        str[i+1] = 0;
    }
    else{
        str[i] = 0;
    }

    int j, k, temp;
    for(j = 0, k = strlen(str) - 1; j < k; j++, k--){
        temp = str[j];
        str[j] = str[k];
        str[k] = temp;
    }

    return;
}

int strlen(char str[]){
    int counter = 0;
    while(str[counter] != 0){
        counter++;
    }
    return counter;
}

int strcmp(char str1[], char str2[]){
    int i = 0;

    do{
        if(str1[i] > str2[i]){
            return 1;
        }
        else if(str1[i] < str2[i]){
            return -1;
        }
        i++;
    } while (str1[i] != 0 && str2[i] != 0);
    
    if(str1[i] > str2[i]){
        return 1;
    }
    else if(str1[i] < str2[i]){
        return -1;
    }

    return 0;
}

void strcpy(char dest[], char src[]){
    int counter = 0;
    while(src[counter] != 0){
        dest[counter] = src[counter];
        counter++;
    }
    dest[counter] = 0;
    return;
}

string_t str_new(char* initial){
    string_t retval;

    int counter = 0;
    while(initial[counter] != 0){
        counter++;
    }
    retval.content = kmalloc (counter + 1);

    for(int i = 0; i < counter; i++){
        retval.content[i] = initial[i];
    }
    retval.content[counter] = 0;
    retval.len = counter;

    return retval;
}

string_t str_newcopy(string_t source){
    string_t retval;

    retval.content = kmalloc (source.len + 1);

    for(uint32_t i = 0; i < source.len; i++){
        retval.content[i] = source.content[i];
    }
    retval.content[source.len] = 0;
    retval.len = source.len;

    return retval;
}

string_t str_splice_rear(string_t mainstring, uint32_t loc){
    string_t retval;
    retval.len = mainstring.len - loc;

    retval.content = kmalloc (retval.len + 1);

    for(uint32_t i = 0; i < retval.len; i++){
        retval.content[i] = mainstring.content[loc + i];
    }
    retval.content[retval.len] = 0;
    return retval;
}

string_t str_splice_front(string_t mainstring, uint32_t loc){
    string_t retval;
    retval.len = loc;

    retval.content = kmalloc (retval.len + 1);

    for(uint32_t i = 0; i < retval.len; i++){
        retval.content[i] = mainstring.content[i];
    }
    retval.content[retval.len] = 0;
    return retval;
}

void str_delete(string_t* string){
    kfree(string->content);
}

void str_concat(string_t* mainstring, string_t substring){
    mainstring-> content = krealloc(mainstring->content, mainstring->len + substring.len + 1);

    for(uint32_t i = 0; i < substring.len; i++){
        mainstring->content[mainstring->len + i] = substring.content[i];
    }
    mainstring->len += substring.len;
    mainstring->content[mainstring->len] = 0;
}

void str_consdot(string_t* mainstring, string_t substring){
    mainstring-> content = krealloc(mainstring->content, mainstring->len + substring.len + 1);
    string_t temp = str_newcopy(*mainstring);

    for(uint32_t i = 0; i < substring.len; i++){
        mainstring->content[i] = substring.content[i];
    }
    for(uint32_t i = 0; i < mainstring->len; i++){
        mainstring->content[substring.len + i] = temp.content[i];
    }
    str_delete(&temp);
    mainstring->len += substring.len;
    mainstring->content[mainstring->len] = 0;
}

void str_insertc(string_t* mainstring, char c, uint32_t loc){
    mainstring-> content = krealloc(mainstring->content, mainstring->len + 2);
    for(uint32_t i = mainstring->len; i > loc; i--){
        mainstring->content[i] = mainstring->content[i-1];
    }
    mainstring->content[loc] = c;
    mainstring->content[mainstring->len + 1] = 0;
    mainstring->len++;
}

char str_remove(string_t* mainstring, uint32_t loc){
    char retval = mainstring->content[loc];

    for(uint32_t i = loc; i < mainstring->len; i++){
        mainstring->content[i] = mainstring->content[i + 1];
    }
    mainstring->content[mainstring->len - 1] = 0;
    mainstring->content = krealloc(mainstring->content, mainstring->len);
    mainstring->len--;

    return retval;
}

void str_add(string_t* mainstring, char* substring){
    string_t temp = str_new(substring);
    str_concat(mainstring, temp);
    str_delete(&temp);
}

void str_addc(string_t* mainstring, char c){
    mainstring-> content = krealloc(mainstring->content, mainstring->len + 2);
    mainstring->content[mainstring->len] = c;
    mainstring->content[mainstring->len + 1] = 0;
    mainstring->len++;
}