# reset state of application when F5 is pressed

# STATUS: OPEN
# PRIORITY: 70

When we wanto to refresh the application, we need to
- reset (delete all items and allocations) of the render_list
- reload the md file
- parse md file again (which will prepare the render_list with the new parsing of the md file)
