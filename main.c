#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <stdbool.h>
#include <sys/stat.h>

#define MAX_FILES 5 // the max number of files to track
#define EXTENSION ".md"

static char *vault_path = NULL;
static char *main_notes_path = NULL;
static char *fav_folder_one = "5-MainNotes";  // Keep it as a string
static char *skip_folder = "Kaizen Journaling";

typedef struct {
  char name[256];
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
  } else {
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
    char full_path[512];
    snprintf(full_path, sizeof(full_path), "%s/%s", f_dir, entry->d_name);

    if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0 || strstr(full_path, skip_folder) != NULL) {
      continue;
    }

    struct stat file_stat;

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
  return (int)(((FileEntry *)b)->last_access - ((FileEntry *)a)->last_access);
}

void display_recent_files() {
  qsort(files, count, sizeof(FileEntry), compare_files);

  printf("\nLast %d accessed files:\n", MAX_FILES);
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

  // Here you allocate memory for vault_path and main_notes_path, but don't touch fav_folder_one
  size_t len = strlen(home) + strlen("/obsidian/scnd-brain/") + 1;
  vault_path = malloc(len);
  size_t len2 = strlen(home) + strlen("/obsidian/scnd-brain/") + strlen(fav_folder_one) + 1;
  main_notes_path = malloc(len2);
  if (!vault_path || !main_notes_path) {
    fprintf(stderr, "Memory allocation failed.");
    exit(1);
  }

  snprintf(vault_path, len, "%s/obsidian/scnd-brain/", home);
  snprintf(main_notes_path, len2, "%s/obsidian/scnd-brain/%s", home, fav_folder_one);
}

void cleanup() {
  free(vault_path);
  free(main_notes_path);
}

int get_user_input() {
  int action;

  printf("\nSelect an option: \n");
  printf("1: create a new file\n");
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

  // Create the file in the correct path without modifying fav_folder_one directly
  char command[256]; 
  snprintf(command, sizeof(command), "nvim %s/%s.md", main_notes_path, nufile);

  strncpy(nufile, command, size - 1);
  nufile[size - 1] = '\0';
}

int main() {
  init_vault_path();
  int action;

  while (true) {
    printf("Welcome back!\n");

    list_recent_files(vault_path);
    display_recent_files();

    action = get_user_input();

    if (action == -1) {
      continue;
    }

    switch(action){
      case 1:
        // Open a folder
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

    cleanup();
    init_vault_path();  // Re-initialize paths before continuing the loop
  }
  return 0;
}

