#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <stdbool.h>
#include <sys/stat.h>

#define MAX_FILES 5 // the max number of files to track
#define EXTENSION ".md"

static char *vault_path = NULL;
static char *fav_folder_one = "5-MainNotes";
static char *fav_folder_two = "1-SourceMaterial";
static char *skip_folder = "Kaizen Journaling";

typedef struct {
  char name [256];
  time_t last_access;
} FileEntry;

FileEntry files[MAX_FILES];
int count = 0;

bool is_md_file(const char *filename) {
  const char *ext = strrchr(filename, '.');
  return (ext && strcmp(ext, EXTENSION) == 0);
}

void update_recent_files(const char *filepath, time_t access_time){
  if (count < MAX_FILES) {
    strncpy(files[count].name, filepath, sizeof(files[count].name) - 1);
    files[count].name[sizeof(files[count].name) - 1] = '\0'; // Ensure null termination
    files[count].last_access = access_time;
    count++;
  } else{
    int oldest_index = 0;
    for (int i = 1; i < MAX_FILES; i++) {
      if (files[i].last_access < files[oldest_index].last_access) {
        oldest_index = i;
      }
    }

    if (access_time > files[oldest_index].last_access) {
      strncpy(files[oldest_index].name, filepath, sizeof(files[oldest_index].name) - 1);
      files[oldest_index].name[sizeof(files[oldest_index].name) - 1] = '\0';
      files[oldest_index].last_access = access_time;
    }
  }
}

void list_recent_files(const char *f_dir) {
  struct dirent *entry;
  DIR *dir = opendir(f_dir);
  if (!dir) {
      perror("Unable to open directory.");
      return;
  }

  while ((entry = readdir(dir)) != NULL) {
    if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0 || strcmp(entry->d_name, skip_folder) == 0) {
      continue;
    }

    char full_path[512];
    snprintf(full_path, sizeof(full_path), "%s/%s", f_dir, entry->d_name);

    struct stat file_stat;
    char subdir[512];

    if (stat(full_path, &file_stat) == 0) {
      if(S_ISREG(file_stat.st_mode) && is_md_file(entry->d_name)) {
        update_recent_files(full_path, file_stat.st_atime);
      } else if (S_ISDIR(file_stat.st_mode)) {
        list_recent_files(full_path);
      }
    }
  }
  closedir(dir);
}

int compare_files(const void *a, const void *b) {
  return ((FileEntry *)b)->last_access - ((FileEntry *)a)->last_access;
}

void display_recent_files() {
  qsort(files, count, sizeof(FileEntry), compare_files);

  printf("\nLast %d accesed files:\n", MAX_FILES);
  for(int i = 0; i < count; i++) {
    printf("%d. %s\n", i+1, files[i].name);
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

int get_user_input() {
  int action;

  printf("\nSelect an option: \n");
  printf("1: open first favourite folder\n");
  printf("0: quit\n");
  printf("> ");

  if (scanf("%d", &action) != 1) {
    printf("Invalid input, try with a number.\n");
    while(getchar() != '\n');
    return -1;
  }
  if (action == 0 || action == 1) {
    return action;
  } else {
    printf("Invalid option. Try again\n");
    return -1;
  }
}

void create_file(char *nufile, size_t size) {
  printf("Insert name of the file: ");
  if (fgets(nufile, size, stdin) == NULL) {
    printf("Error reading file. Try again\n");
    return;
  }

  nufile[strcspn(nufile, "\n")] = 0;

  char temp[256];
  snprintf(temp, sizeof(temp), "nvim %s.md", nufile);

  strncpy(nufile, temp, size - 1);
  nufile[size - 1] = '\0';
}

int main() {
  init_vault_path();
  int action;

  char main_notes_path[512];
  snprintf(main_notes_path, sizeof(main_notes_path), "%s/%s", vault_path, fav_folder_one);

  while (true) {
    printf("Welcome back!\n"); 
    list_fav_dirs(vault_path);

    // List your accessed files
    // list_recent_files(main_notes_path);
    list_recent_files(vault_path);
    display_recent_files();

    action = get_user_input();

    if (action == -1) {
      continue;
    }

    switch(action){
      case 1:
      //open a folder
        char nufile[256];
        create_file(nufile, sizeof(nufile));
        printf("Running command: %s\n", nufile);
        system(nufile);
        cleanup();
        exit(0);
      case 0:
        system("clear");
        printf("Good luck!\n");
        cleanup();
        exit(0);
      default:
        printf("You didn't choose a valid option");
    }
  }
  return 0;
}
