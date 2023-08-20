#!/usr/bin/env python3
# -*- coding:utf-8 -*-

# This script processes Tiled maps before they can be processed by rescomp
# For now it just makes sure that, in the case of graphics in the objects layer,
# the indirect properties from the object's "gid" are explicitly brought in.

# Otherwise, the OBJECTS rescomp directive will simply skip those objects
# It also ensure that whatever objects have a true "exportDimensions" property,
# will aso have a true "exportSize" property as well, for compat reasons

from lxml import etree
import sys
import os
from typing import Any
from copy import deepcopy

inp = sys.argv[1]
out = sys.argv[2]
basepath = os.path.dirname(inp)
tilesets:list[tuple[str, int]] = []     # tileset relatove filename, and first gid
tileprops:dict[int, Any] = {}           # for every gid, all properties that were found

print(f"Processing '{inp}'...")

tree:etree.ElementTree = etree.parse(inp)
root:etree.ElementBase = tree.getroot()

# first, list all tilesets based on gid
for elem in root.findall("tileset"):
    elem:etree.ElementBase
    path = elem.get("source")
    firstgid = elem.get("firstgid")
    if not (path and firstgid):
        raise ValueError()
    tilesets.append((path, int(firstgid),))

# parse every tileset and store reference to properties for every gid
for path, fgid in tilesets:
    path = os.path.join(basepath, path)
    print(f"\tReading tileset {os.path.basename(path)}")
    t_tree:etree.ElementTree = etree.parse(path)
    t_root:etree.ElementBase = t_tree.getroot()
    for tile in t_root.findall("tile"):
        props_elem = tile.find("properties")
        if props_elem is not None:
            gid = fgid + int(tile.get("id"))
            #print("TILE", gid)
            tileprops[gid] = props_elem

# then for every objects layer and object
for objgrp in root.findall("objectgroup"):
    for objelem in objgrp.findall("object"):
        # look for objects with gid and no properties
        objelem:etree.ElementBase
        #print(etree.tostring(objelem), "gid" in objelem.attrib)
        if ("gid" in objelem.attrib) and (objelem.find("properties") is None):
            gid = int(objelem.get('gid'))
            # if gid exists, copy properties to object
            #print(f"\tFound object {objelem.get('id')}, no props, gid {gid}")
            if gid in tileprops:
                #print("UPDATED")
                objelem.append(deepcopy(tileprops[gid]))
                # finally, remove gid because rescomp is picky
                objelem.attrib.pop("gid")
        # in any case, always check if propery exportSize is same as exportDimensions
        props_elem = objelem.find("properties")
        if props_elem is not None:
            props_elem:etree.ElementBase
            dims_prop = props_elem.find("property[@name = 'exportDimensions']")
            if dims_prop is not None:
                size_prop = props_elem.find("property[@name = 'exportSize']")
                if size_prop is None:
                    props_elem.append(etree.fromstring(b'<property name="exportSize" type="bool" value="true"/>'))
                else:
                    size_prop.set("value", dims_prop.get("value"))
            

# save final file
f = open(out, 'wb')
f.write(etree.tostring(root, pretty_print=True))
f.close()
print(f"\tWrote to '{out}'")