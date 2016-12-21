/**
 * Welcome to Seashell!
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct file{//a standardized struct for containing a file. includes a type
  int type;
  char* name;
  void* content;
  struct file* par;
};

struct bstaugnode{//stores files and directories in a binary search tree
  struct file* item;
  struct bstaugnode* left;
  struct bstaugnode* right;
};//void pointer allows this to be anything. another augnode/directory or a file type

struct augnode{//the name of the item and then binary search tree. used for directory
  struct bstaugnode* fsys;
};

struct fbyte{//a primitive file type
  int size;
  char* content;//technically any file can be represented with dynamic # of bytes
};

char* readline(){
  char *str=malloc(sizeof(char));
  char c;
  int length=0;
  int size=1;
  while(1){//doubling strategy
    int buf=scanf("%c",&c);
    if(buf==0||buf==EOF||c==' '||c=='\n')
      break;
    if(length+1==size){
      size*=2;
      str=realloc(str,sizeof(char)*size);
    }
    str[length++]=c;
  }
  str[length++]=0;
  str=realloc(str,sizeof(char)*length);
  return str;
}

int sistr(const char* str){
  int length=0;
  while(str[length++]);
  return length;
}

struct file* mknode(const char* ns, struct file* parent){
  struct file* f=malloc(sizeof(struct file));
  f->type=0;
  struct augnode* n=malloc(sizeof(struct augnode));
  f->name=malloc(sizeof(char)*sistr(ns));
  strcpy(f->name,ns);
  n->fsys=NULL;
  f->par=parent;
  f->content=n;
  return f;
}

//decided to separate the bstsearch into two segments to improve memory usage
struct bstaugnode* bstsearchminus(struct bstaugnode* curr,const char* nm){
  if(curr){
    int result=strcmp(nm,curr->item->name);
    if(result<0)
      curr->left=bstsearchminus(curr->left,nm);
    else if(result>0)
      curr->right=bstsearchminus(curr->right,nm);
    else{
      if(curr->item->type){
        if(curr->item->type>0){
          curr->right=bstsearchminus(curr->right,nm);
        }
        else{
          curr->left=bstsearchminus(curr->left,nm);
        }
      }
      else{
        if (((struct augnode*)(curr->item->content))->fsys){
          printf("Warning. Your target folder isn't empty.");
          //printf("Delete all content? Y/N\n");
        //TODO: Actually do the deletion
        }
        else{
          struct bstaugnode* rp=curr->left;
          if(rp){
            if(rp->right){
              while(rp->right->right)rp=rp->right;
              free(curr->item->name);
              free(curr->item->content);
              free(curr->item);
              struct bstaugnode* bak=rp->right;
              bak->right=curr->right;
              rp->right=bak->left;
              bak->left=curr->left;
              free(curr);
              curr=bak;
            }
            else{
              free(curr->item->name);
              free(curr->item->content);
              free(curr->item);
              rp->right=curr->right;
              free(curr);
              curr=rp;
            }
          }
          else if(curr->right){
            struct bstaugnode* bak=curr->right;
            free(curr->item->name);
            free(curr->item->content);
            free(curr->item);
            free(curr);
            curr=bak;
          }
        }
      }
    }
  }
  else{
    printf("No directory by this name found.");
  }
  return curr;
}

struct bstaugnode* bstsearchplus(struct bstaugnode* curr, struct file* par,const char* nm){
  if(curr){
    int result=strcmp(nm,curr->item->name);
    if(result<0)
      curr->left=bstsearchplus(curr->left,par,nm);
    else if(result>0)
      curr->right=bstsearchplus(curr->right,par,nm);
    else{
      if(curr->item->type){
        if(curr->item->type>0)
          curr->right=bstsearchplus(curr->right,par,nm);
        else{
          curr->left=bstsearchplus(curr->left,par,nm);
        }
      }
      else{
        printf("Directory already exists.");
      }
    }
  }
  else{
    curr=malloc(sizeof(struct bstaugnode));
    curr->item=mknode(nm,par);
    curr->left=NULL;
    curr->right=NULL;
  }
  return curr;
}


void mkdir(struct file* curr){
  char* nm=readline();
  ((struct augnode*)(curr->content))->fsys=bstsearchplus(((struct augnode*)(curr->content))->fsys,curr,nm);
  free(nm);
}

void rmdir(struct file* curr){
  char* nm=readline();
  ((struct augnode*)(curr->content))->fsys=bstsearchminus(((struct augnode*)(curr->content))->fsys,nm);
  free(nm);
}

void lsprint(struct bstaugnode* curr){
  if(curr){
    lsprint(curr->left);
    printf("%s\t",curr->item->name);
    lsprint(curr->right);
  }
}

//due to binary search tree the print can be only alphabetical right now
void ls(struct file* curr){
  lsprint(((struct augnode*)(curr->content))->fsys);
  printf("\n");
}

void pwd(struct file* curr){
  if(curr->par==NULL){
    printf("://%s",curr->name);
    return;
  }
  pwd(curr->par);
  printf("/%s",curr->name);
}

struct file* bstsearch(struct bstaugnode* curr,const char* nm){
  if(curr){
    int result=strcmp(nm,curr->item->name);
    if(result<0)
      return bstsearch(curr->left,nm);
    else if(result>0)
      return bstsearch(curr->right,nm);
    else{
      if(curr->item->type){
        if(curr->item->type>0){
          return bstsearch(curr->right,nm);
        }
        else{
          return bstsearch(curr->left,nm);
        }
        
      }
      else{
        return curr->item;
      }
    }
  }
  else{
    printf("Directory not found.");
    return NULL;
  }
}

struct file* cd(struct file* curr){
  char* nm=readline();
  if(strcmp("..",nm)){
    struct file* t = bstsearch(((struct augnode*)(curr->content))->fsys,nm);
    free(nm);
    return t?t:curr;
  }
  else{
    free(nm);
    return curr->par?curr->par:curr;
  }
}

//scalable command system. uses a binary search to navigate between a sorted list of applicable command
//cd and quit being special cases is left out.
//the benefits may not be immediate in this instance. but if you have 100s of commands, this will result in logn versus n level of speed improvement
char** instr;
void (**funs)(struct file*);
int numoffun=4;

void emp(struct file* curr){

}

void init(void){
  instr=malloc((numoffun+1)*(sizeof(char*)));
  funs=malloc((numoffun+1)*(sizeof(void(*)(struct file*))));
  funs[0]=emp;
  instr[1]="ls";
  funs[1]=ls;
  instr[2]="mkdir";
  funs[2]=mkdir;
  instr[3]="pwd";
  funs[3]=pwd;
  instr[4]="rmdir";
  funs[4]=rmdir;
}

int binstr(const char* cmd){
  int ub=numoffun,lb=1;
  int an,mp;
  while(ub-lb>1){
    mp=(ub+lb)/2;
    an=strcmp(cmd,instr[mp]);
    if(an>0){
      lb=mp;
    }
    else if(an<0){
      ub=mp;
    }
    else{
      return mp;
    }
  }

    if(strcmp(cmd,instr[ub])){
      if(ub==lb){
        return 0;
      }
      else{
        if(strcmp(cmd,instr[lb])){
          return 0;
        }
        else{
          return lb;
        }
      }
    }
    else{
      return ub;
    }
    
}

int main(void) {
  init();
  char* cmd;
  struct file* curr=mknode("root",NULL);
  while(1){
    cmd=readline();
    if(strcmp(cmd,"cd")==0){
      curr=cd(curr);
    }
    else if(strcmp(cmd,"quit")==0){
      break;
    }
    else{
      funs[binstr(cmd)](curr);
    }
    printf("\n");
    free(cmd);
  }
}
