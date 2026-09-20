#include "lc_memory.h"

#include <ctype.h>
#include <stdio.h>
#include <string.h>

#define LC_MEMORY_PATH_MAX 512u
#define LC_MEMORY_FILE_MAX 256L
#define LC_MEMORY_ALLERGEN_MASK \
  (LC_ALLERGEN_PEANUT | LC_ALLERGEN_DAIRY | LC_ALLERGEN_EGG | \
   LC_ALLERGEN_SHELLFISH)

void lc_memory_init(lc_memory_t *memory)
{
  if (memory == NULL)
    {
      return;
    }

  memset(memory, 0, sizeof(*memory));
  memory->version = LC_MEMORY_VERSION;
}

bool lc_memory_apply_confirmation(lc_memory_t *memory,
                                  const lc_memory_update_t *update)
{
  bool changed = false;

  if (memory == NULL || update == NULL || !update->confirmed)
    {
      return false;
    }

  if (update->remember_budget &&
      (!memory->has_budget || memory->budget_cents != update->budget_cents))
    {
      memory->has_budget = true;
      memory->budget_cents = update->budget_cents;
      changed = true;
    }

  if (update->remember_allergens)
    {
      unsigned int allergen_flags = update->allergen_flags &
                                    LC_MEMORY_ALLERGEN_MASK;

      if (memory->allergen_flags != allergen_flags)
        {
          memory->allergen_flags = allergen_flags;
          changed = true;
        }
    }

  return changed;
}

void lc_memory_clear_budget(lc_memory_t *memory)
{
  if (memory == NULL)
    {
      return;
    }

  memory->has_budget = false;
  memory->budget_cents = 0u;
}

void lc_memory_clear_all(lc_memory_t *memory)
{
  lc_memory_init(memory);
}

lc_memory_status_t lc_memory_save(const char *path, const lc_memory_t *memory)
{
  char temporary_path[LC_MEMORY_PATH_MAX];
  FILE *file;
  int written;

  if (path == NULL || memory == NULL ||
      memory->version != LC_MEMORY_VERSION ||
      (memory->allergen_flags & ~LC_MEMORY_ALLERGEN_MASK) != 0u)
    {
      return LC_MEMORY_MALFORMED;
    }

  written = snprintf(temporary_path, sizeof(temporary_path), "%s.tmp", path);
  if (written < 0 || (size_t)written >= sizeof(temporary_path))
    {
      return LC_MEMORY_IO_ERROR;
    }

  file = fopen(temporary_path, "w");
  if (file == NULL)
    {
      return LC_MEMORY_IO_ERROR;
    }

  if (fprintf(file,
              "LCM%u\nbudget_known=%u\nbudget=%u\nallergens=%u\n",
              memory->version,
              memory->has_budget ? 1u : 0u,
              memory->budget_cents,
              memory->allergen_flags) < 0 ||
      fflush(file) != 0 || fclose(file) != 0)
    {
      remove(temporary_path);
      return LC_MEMORY_IO_ERROR;
    }

  if (rename(temporary_path, path) != 0)
    {
      remove(temporary_path);
      return LC_MEMORY_IO_ERROR;
    }

  return LC_MEMORY_OK;
}

lc_memory_status_t lc_memory_load(const char *path, lc_memory_t *memory)
{
  FILE *file;
  long size;
  unsigned int version;
  unsigned int budget_known;
  unsigned int budget;
  unsigned int allergens;
  int parsed;
  int character;
  bool trailing_data = false;

  if (path == NULL || memory == NULL)
    {
      return LC_MEMORY_MALFORMED;
    }

  file = fopen(path, "r");
  if (file == NULL)
    {
      return LC_MEMORY_IO_ERROR;
    }

  if (fseek(file, 0L, SEEK_END) != 0 ||
      (size = ftell(file)) < 0L || size > LC_MEMORY_FILE_MAX ||
      fseek(file, 0L, SEEK_SET) != 0)
    {
      fclose(file);
      return LC_MEMORY_MALFORMED;
    }

  parsed = fscanf(file,
                  "LCM%u\nbudget_known=%u\nbudget=%u\nallergens=%u\n",
                  &version, &budget_known, &budget, &allergens);
  while ((character = fgetc(file)) != EOF)
    {
      if (!isspace((unsigned char)character))
        {
          trailing_data = true;
          break;
        }
    }

  if (ferror(file) != 0)
    {
      fclose(file);
      return LC_MEMORY_IO_ERROR;
    }

  if (fclose(file) != 0)
    {
      return LC_MEMORY_IO_ERROR;
    }

  if (parsed != 4 || trailing_data)
    {
      return LC_MEMORY_MALFORMED;
    }

  if (version != LC_MEMORY_VERSION)
    {
      return LC_MEMORY_UNSUPPORTED_VERSION;
    }

  if (budget_known > 1u ||
      (allergens & ~LC_MEMORY_ALLERGEN_MASK) != 0u)
    {
      return LC_MEMORY_MALFORMED;
    }

  lc_memory_init(memory);
  memory->has_budget = budget_known == 1u;
  memory->budget_cents = memory->has_budget ? budget : 0u;
  memory->allergen_flags = allergens;
  return LC_MEMORY_OK;
}
