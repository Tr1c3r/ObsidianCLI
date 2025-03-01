#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <stdbool.h>

static char *vault_path = NULL;
static char *fav_folder_one = "5-MainNotes";
static char *fav_folder_two = "1-SourceMaterial";

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
  cleanup();
  return 0;
}
