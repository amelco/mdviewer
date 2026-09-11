# find a way to not dup the text

# STATUS: OPEN
# PRIORITY: 70

We are allocating and deallocating memory on heap too fast. This is not an optimal solution.
Find a way of getting the new string for each iteration without using strdup (maybe using some sort of string_view?)
