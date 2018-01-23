ding_custom_facets
=========================================
This module add custom facet groups functionality, where editors can create new groups and define the terms within the groups.
This module is presented as a panel pane "Custom facet groups + terms" in "Ding!" group.
An unlimited number of groups can be added and rendered.

## Installation
- Enable 'Ding Custom Facet Groups' module on /admin/modules page. 

## Configuration
- Go to /admin/config/ding/facet-groups
- Click "Add new group" tab or got to /admin/config/ding/facet-groups/add
- Insert desired data in 'Group name' field a nd click button 'Save'. (After redirection you will see newly created group in groups table.)
- On "Manage custom facet groups" page click on group name you've just created
- On Group's page you should see term table with "Label" and "CQL query" (See example below) fields where you have to fill your data click "Save" button.
- Go to "Ting Search" pane and add "Custom facet groups + terms" pane from "Ding!" group to desired region.
- On the pane settings form check group/groups which should be shown.

#### Example
For example create a group called "Various" and add 2 terms:
- Label: Can be accessed online (CQL query: "AND term.accessType=online")
- Label: Not library items (CQL query: "NOT term.acSource=bibliotekskatalog")
