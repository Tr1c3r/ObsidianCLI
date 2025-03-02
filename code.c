#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <stdbool.h>
#include <sys/stat.h>

#define MAX_FILES 5 // the max number of files to track

static char *vault_path = NULL;
static char *fav_folder_one = "5-MainNotes";
static char *fav_folder_two = "1-SourceMaterial";

typedef struct {
  char name [256];
  time_t last_access;
} FileEntry;

void list_recent_files(const char *f_dir) {
  struct dirent *entry;
  DIR *dir = opendir(path);
  if (!dir) {
    perror("Unable to open directory.");
    return;
  }

  FileEntry files[MAX_FILES];
  int count = 0;

  while((entry = readdir(dir)) != NULL) {
    if (entry->d_type == DT_REG) {
      //char full_path[512];
      snprintf(full_path, sizeof(full_path) + sizeof(vault_path), f_dir, entry->d_name);

      struct stat file_stat;
      if (stat(full_path, &file_stat) == 0) {
        strncpy(files[count].name, entry->d_name, sizeof(files[count].name) - 1);
        files[count].last_access = file_stat.st_atime;
        count ++;
        if (count >= MAX_FILES) break;
      }
    }
  }

  closedir(dir);
  
  for (int i = 0; i < count - 1; i++) {
    for (int j = i + 1; j < count; j++) {
      if (files[i].last_access < files[j].last_access) {
        FileEntry temp = files[i];
        files[i] = files[j];
        files[j] = temp;
      }
    }
  }

  printf("Recently accessed files: \n");
  for (int i = 0; i < count; i++) {
    printf("[FILE] %s (Last accessed: %ld)\n", i+1, files[i].name, files[i].last_access);
  }
}

void init_vault_path() {
  const char *home = getenv("HOME");
  if (!home) {
    fprintf(stderr, "Error: HOME environment variable not set.");
    exit(1);
  }

  // here you would insert your REAL obsidian vault directory.
  size_t len = strlen(home) + strlen("/obsidian/scnd-brain/") + 1;
  vault_path = malloc(len);
  if (!vault_path) {
    fprintf(stderr, "Memory allocation failed.");
    exit(1);
  }

  snprintf(vault_path, len, "%s/obsidian/scnd-brain/", home);
}

void cleanup() {
  free(vault_path);
}

void list_fav_dirs(const char *path) {
  struct dirent *entry;
  DIR *dir = opendir(path);

  if (!dir) {
    perror("Unable to find directory.");
    return;
  }

  printf("Your favourite vault folders: \n");
  while ((entry = readdir(dir)) != NULL) {
    // Enter as many folders as you want to show in this section
    // Rememeber to add the variable at the beginning or hard type it
    if (entry->d_type == DT_DIR &&
    (strcmp(entry->d_name, fav_folder_one) == 0 || strcmp(entry->d_name, fav_folder_two) == 0)) {
      printf("[DIR] %s\n", entry->d_name);
    }
  }

  closedir(dir);

}

int main() {
  int action;
  init_vault_path();

  while (true) {
    printf("Welcome back!\n"); 
    list_fav_dirs(vault_path);
    printf("Select an option: \n");
    printf("1: open first favourite folder\n");
    printf("0: quit\n");
    scanf("%d", &action);

    switch(action){
      case 1:
      //open a folder
        printf("You opened a folder.");
        break;
      case 0:
        system("clear");
        printf("Good luck!\n");
        cleanup();
        exit(0);
      default:
        printf("You didn't choose a valid option");
    }
  }
  list_recent_files(vault_path);
  cleanup();
  return 0;
}
