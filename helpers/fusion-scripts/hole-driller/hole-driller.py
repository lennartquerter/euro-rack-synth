# Author: Lennart Querter
# Description: This script can be used to generate euro-rack front-panels based on JSON files stored in this repository.
import adsk.core, adsk.fusion, adsk.cam, traceback
import os
import json

full_height = 12.85
fillet_size = 0.12
panel_offset_z = 0.2
hp_to_mm = 5.08

hole_size_map = {
    "jacks": 0.62,
    "pots": 0.72,
    "switches": 0.65,
    "leds": 0.50,
    "sm_leds": 0.30,
}


def run(context):
    ui = None

    try:
        app = adsk.core.Application.get()
        ui = app.userInterface
        design = app.activeProduct
        root_comp = design.rootComponent

        module_name, cancelled = ui.inputBox(
            'Module name?',
            'module',
            'example')

        if cancelled:
            return 1

        with open(os.path.dirname(os.path.realpath(__file__)) + "/modules/" + module_name + '.json') as f:
            d = json.load(f)

            create_panel(root_comp, d["hp"])
            add_mounting_holes(root_comp, d["hp"])

            add_points(root_comp, d, 'pots')
            add_points(root_comp, d, 'jacks')
            add_points(root_comp, d, 'switches')
            add_points(root_comp, d, 'leds')
            add_points(root_comp, d, 'sm_leds')

    except:
        if ui:
            ui.messageBox('Failed:\n{}'.format(traceback.format_exc()))


def create_panel(root_comp, hp_size):
    # Create a new sketch on the xy plane.
    sketches = root_comp.sketches
    xyPlane = root_comp.xYConstructionPlane
    sketch = sketches.add(xyPlane)
    sketch.name = 'front-panel'

    lines = sketch.sketchCurves.sketchLines

    lines.addTwoPointRectangle(adsk.core.Point3D.create(0, 0, 0),
                               adsk.core.Point3D.create((hp_size * hp_to_mm) / 10, full_height, 0))

    panel = extrude(root_comp, sketch.profiles.item(0), panel_offset_z,
                    adsk.fusion.FeatureOperations.NewBodyFeatureOperation)
    body = panel.bodies.item(0)
    body.name = "panel"


def extrude(root_comp, profile, distance, operation):
    distance = adsk.core.ValueInput.createByReal(distance)
    extrudes = root_comp.features.extrudeFeatures

    return extrudes.addSimple(profile, distance, operation)


def add_mounting_holes(root_comp, hp_size):
    sketches = root_comp.sketches
    xyPlane = root_comp.xYConstructionPlane
    sketch = sketches.add(xyPlane)
    sketch.name = 'mounting-holes'

    lines = sketch.sketchCurves.sketchLines

    full_width = (hp_size * 5.08) / 10

    # CONFIG
    # ==============
    # X Axis
    side_margin = 0.45
    hole_width = 0.8

    # Y Axis
    height_margin = 0.2
    hole_height = 0.32

    if hp_size == 2:
        side_margin = 0.204
        hole_width = 0.608


    # ============== Create Mounting holes

    mounting_hole_1 = lines.addTwoPointRectangle(adsk.core.Point3D.create(side_margin, height_margin, panel_offset_z),
                                                 adsk.core.Point3D.create(side_margin + hole_width,
                                                                          height_margin + hole_height, panel_offset_z))

    add_fillet(sketch, mounting_hole_1, fillet_size)

    extrude(root_comp, sketch.profiles.item(0), -panel_offset_z,
            adsk.fusion.FeatureOperations.CutFeatureOperation)

    mounting_hole_2 = lines.addTwoPointRectangle(
        adsk.core.Point3D.create(full_width - side_margin, full_height - height_margin, panel_offset_z),
        adsk.core.Point3D.create(full_width - (side_margin + hole_width), full_height - (height_margin + hole_height),
                                 panel_offset_z))

    add_fillet(sketch, mounting_hole_2, fillet_size)

    extrude(root_comp, sketch.profiles.item(1), -panel_offset_z,
            adsk.fusion.FeatureOperations.CutFeatureOperation)

    if hp_size >= 10:
        extra_hole_sketch = sketches.add(xyPlane)
        extra_hole_sketch.name = 'mounting-holes-extra'

        lines = extra_hole_sketch.sketchCurves.sketchLines

        mounting_hole_3 = lines.addTwoPointRectangle(
            adsk.core.Point3D.create(full_width - side_margin, height_margin, panel_offset_z),
            adsk.core.Point3D.create(full_width - (side_margin + hole_width), (height_margin + hole_height),
                                     panel_offset_z))

        add_fillet(extra_hole_sketch, mounting_hole_3, fillet_size)

        extrude(root_comp, extra_hole_sketch.profiles.item(0), -panel_offset_z,
                adsk.fusion.FeatureOperations.CutFeatureOperation)

        mounting_hole_4 = lines.addTwoPointRectangle(
            adsk.core.Point3D.create(side_margin, full_height - height_margin, panel_offset_z),
            adsk.core.Point3D.create(side_margin + hole_width, full_height - (height_margin + hole_height),
                                     panel_offset_z))

        add_fillet(extra_hole_sketch, mounting_hole_4, fillet_size)

        extrude(root_comp, extra_hole_sketch.profiles.item(1), -panel_offset_z,
                adsk.fusion.FeatureOperations.CutFeatureOperation)


def add_fillet(sketch, rectangle, size):
    sketch.sketchCurves.sketchArcs.addFillet(rectangle.item(1),
                                             rectangle.item(1).startSketchPoint.geometry,
                                             rectangle.item(0),
                                             rectangle.item(0).startSketchPoint.geometry,
                                             size)

    sketch.sketchCurves.sketchArcs.addFillet(rectangle.item(2),
                                             rectangle.item(2).startSketchPoint.geometry,
                                             rectangle.item(1),
                                             rectangle.item(1).startSketchPoint.geometry,
                                             size)

    sketch.sketchCurves.sketchArcs.addFillet(rectangle.item(3),
                                             rectangle.item(3).startSketchPoint.geometry,
                                             rectangle.item(2),
                                             rectangle.item(2).startSketchPoint.geometry,
                                             size)

    sketch.sketchCurves.sketchArcs.addFillet(rectangle.item(0),
                                             rectangle.item(0).startSketchPoint.geometry,
                                             rectangle.item(3),
                                             rectangle.item(3).startSketchPoint.geometry,
                                             size)


def add_points(root_comp, module_file, type):
    sketches = root_comp.sketches
    xyPlane = root_comp.xYConstructionPlane

    sketch = sketches.add(xyPlane)
    sketch.name = type
    circles = sketch.sketchCurves.sketchCircles

    hole_positions = module_file[type]
    hole_size = hole_size_map[type]

    panel_width_in_mm = hp_to_mm * module_file['hp']
    panel_height_in_mm = full_height * 10

    pcb_width_in_mm = module_file['pcb_width_in_mm']
    pcb_height_in_mm = module_file['pcb_height_in_mm']

    right_offset_in_mm = (panel_width_in_mm - pcb_width_in_mm) / 2
    top_offset_in_mm = (panel_height_in_mm - pcb_height_in_mm) / 2

    for idx, hole in enumerate(hole_positions):
        x = panel_width_in_mm - hole["fromRight"] - right_offset_in_mm
        y = panel_height_in_mm - hole["fromTop"] - top_offset_in_mm

        center_point = adsk.core.Point3D.create(x / 10, y / 10, panel_offset_z)
        circles.addByCenterRadius(center_point, hole_size / 2)

        extrude(root_comp, sketch.profiles.item(idx), -panel_offset_z,
                adsk.fusion.FeatureOperations.CutFeatureOperation)
