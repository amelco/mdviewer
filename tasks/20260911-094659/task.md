# get the maximum height in all objects of the previous line

# STATUS: OPEN
# PRIORITY: 70

Text is always rendered in a new line. We have to get the correct new y value for the cursor so the text can be rendered in the correct height.
Currently, we get the height of the last item in the render_list, but it is not necessaraly the maximum value for the height and the text
can be rendered above some item in the last line.

Find a way to get the maximum height of all items in the line before the text is meant to be rendered.
