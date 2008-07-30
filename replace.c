#include <glib.h>
#include <glib/gprintf.h>


#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
 * Description:
 *   Find and replace text within a string.
 *
 * Parameters:
 *   src  (in) - pointer to source string
 *   from (in) - pointer to search text
 *   to   (in) - pointer to replacement text
 *
 * Returns:
 *   Returns a pointer to dynamically-allocated memory containing string
 *   with occurences of the text pointed to by 'from' replaced by with the
 *   text pointed to by 'to'.
 */
gchar *replace(const gchar *src, const gchar *from, const gchar *to)
{
   /*
    * Find out the lengths of the source string, text to replace, and
    * the replacement text.
    */
   size_t size    = strlen(src) + 1;
   size_t fromlen = strlen(from);
   size_t tolen   = strlen(to);
   /*
    * Allocate the first chunk with enough for the original string.
    */
   gchar *value = g_malloc(size);
   /*
    * We need to return 'value', so let's make a copy to mess around with.
    */
   gchar *dst = value;
   /*
    * Before we begin, let's see if malloc was successful.
    */
   if ( value != NULL )
   {
      /*
       * Loop until no matches are found.
       */
      for ( ;; )
      {
         /*
          * Try to find the search text.
          */
         const gchar *match = g_strstr_len(src, size, from);
         if ( match != NULL )
         {
            /*
             * Found search text at location 'match'. :)
             * Find out how many characters to copy up to the 'match'.
             */
            size_t count = match - src;
            /*
             * We are going to realloc, and for that we will need a
             * temporary pointer for safe usage.
             */
            gchar *temp;
            /*
             * Calculate the total size the string will be after the
             * replacement is performed.
             */
            size += tolen - fromlen;
            /*
             * Attempt to realloc memory for the new size.
             */
            temp = g_realloc(value, size);
            if ( temp == NULL )
            {
               /*
                * Attempt to realloc failed. Free the previously malloc'd
                * memory and return with our tail between our legs. :(
                */
               g_free(value);
               return NULL;
            }
            /*
             * The call to realloc was successful. :) But we'll want to
             * return 'value' eventually, so let's point it to the memory
             * that we are now working with. And let's not forget to point
             * to the right location in the destination as well.
             */
            dst = temp + (dst - value);
            value = temp;
            /*
             * Copy from the source to the point where we matched. Then
             * move the source pointer ahead by the amount we copied. And
             * move the destination pointer ahead by the same amount.
             */
            g_memmove(dst, src, count);
            src += count;
            dst += count;
            /*
             * Now copy in the replacement text 'to' at the position of
             * the match. Adjust the source pointer by the text we replaced.
             * Adjust the destination pointer by the amount of replacement
             * text.
             */
            g_memmove(dst, to, tolen);
            src += fromlen;
            dst += tolen;
         }
         else /* No match found. */
         {
            /*
             * Copy any remaining part of the string. This includes the null
             * termination character.
             */
            strcpy(dst, src);
            break;
         }
      }
   }
   return value;
}

void test(const gchar *source, const gchar *search, const gchar *repl)
{
   gchar *after;
   after = replace(source, search, repl);
   printf("\nsearch = \"%s\", repl = \"%s\"\n", search, repl);
   if ( after != NULL )
   {
      g_printf("after  = \"%s\"\n", after);
      g_free(after);
   }
}

int main(void)
{
   const gchar before[] = "the rain in Spain falls mainly on the plain";
   gchar *z,*x,*y,*uri,*fz, *fx, *fy;

   g_printf("before = \"%s\"\n", before);
   test(before, "the", "THEE");
   test(before, "the", "A");
   test(before, "cat", "DOG");
   test(before, "plain", "PLANE");
   test(before, "ain", "AINLY");

   uri = g_strdup("http://www.foo.com/#Z/#X/#Y-#X-#Z.jpg");
   z = g_strdup_printf("%d", 13);
   x = g_strdup_printf("%d", 100);
   y = g_strdup_printf("%d", 2000);

   fz = replace(uri, "#Z", z);
   fx = replace(fz, "#X", x);
   fy = replace(fx, "#Y", y);

g_printf("after = \"%s\"\n", fy);

    g_free(x);
    g_free(y);
    g_free(z);
    g_free(fx);
    g_free(fy);
    g_free(fz);
    g_free(uri);

   return 0;
}

/* my output
before = "the rain in Spain falls mainly on the plain"

search = "the", repl = "THEE"
after  = "THEE rain in Spain falls mainly on THEE plain"

search = "the", repl = "A"
after  = "A rain in Spain falls mainly on A plain"

search = "cat", repl = "DOG"
after  = "the rain in Spain falls mainly on the plain"

search = "plain", repl = "PLANE"
after  = "the rain in Spain falls mainly on the PLANE"

search = "ain", repl = "AINLY"
after  = "the rAINLY in SpAINLY falls mAINLYly on the plAINLY"
*/
