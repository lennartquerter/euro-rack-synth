# Author: Lennart Querter
# Description: Sets up a Front Panel in autodesk fusion

import adsk.core, adsk.fusion, adsk.cam, traceback

full_height = 12.85
fillet_size = 0.12
panel_offset_z = 0.2


def extrude(root_comp, prof, distance, operation):
    distance = adsk.core.ValueInput.createByReal(distance)
    extrudes = root_comp.features.extrudeFeatures
    return extrudes.addSimple(prof, distance, operation)


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


def create_panel(root_comp, hp_size):
    # Create a new sketch on the xy plane.
    sketches = root_comp.sketches
    xyPlane = root_comp.xYConstructionPlane
    sketch = sketches.add(xyPlane)
    sketch.name = 'front-panel'

    lines = sketch.sketchCurves.sketchLines

    lines.addTwoPointRectangle(adsk.core.Point3D.create(0, 0, 0),
                               adsk.core.Point3D.create((hp_size * 5.08) / 10, 12.85, 0))

    panel_xtrude = extrude(root_comp, sketch.profiles.item(0), 0.2,
                           adsk.fusion.FeatureOperations.NewBodyFeatureOperation)
    body = panel_xtrude.bodies.item(0)
    body.name = "panel"


def add_mounting_holes(root_comp, hp_size):
    sketches = root_comp.sketches
    xyPlane = root_comp.xYConstructionPlane
    sketch = sketches.add(xyPlane)
    sketch.name = 'mounting-holes'

    lines = sketch.sketchCurves.sketchLines

    full_width = (hp_size * 5.08) / 10

    mounting_hole_1 = lines.addTwoPointRectangle(adsk.core.Point3D.create(0.45, 0.2, panel_offset_z),
                                                 adsk.core.Point3D.create(1.25, 0.52, panel_offset_z))

    add_fillet(sketch, mounting_hole_1, fillet_size)

    extrude(root_comp, sketch.profiles.item(0), -0.2,
            adsk.fusion.FeatureOperations.CutFeatureOperation)

    mounting_hole_2 = lines.addTwoPointRectangle(
        adsk.core.Point3D.create(full_width - 0.45, full_height - 0.2, panel_offset_z),
        adsk.core.Point3D.create(full_width - 1.25, full_height - 0.52, panel_offset_z))

    add_fillet(sketch, mounting_hole_2, fillet_size)

    extrude(root_comp, sketch.profiles.item(1), -0.2,
            adsk.fusion.FeatureOperations.CutFeatureOperation)

    if hp_size >= 10:
        mounting_hole_3 = lines.addTwoPointRectangle(adsk.core.Point3D.create(full_width - 0.45, 0.2, panel_offset_z),
                                                     adsk.core.Point3D.create(full_width - 1.25, 0.52, panel_offset_z))

        add_fillet(sketch, mounting_hole_3, fillet_size)

        extrude(root_comp, sketch.profiles.item(2), -0.2,
                adsk.fusion.FeatureOperations.CutFeatureOperation)

        mounting_hole_4 = lines.addTwoPointRectangle(adsk.core.Point3D.create(0.45, full_height - 0.2, panel_offset_z),
                                                     adsk.core.Point3D.create(1.25, full_height - 0.52, panel_offset_z))

        add_fillet(sketch, mounting_hole_4, fillet_size)

        extrude(root_comp, sketch.profiles.item(3), -0.2,
                adsk.fusion.FeatureOperations.CutFeatureOperation)


def add_points(root_comp, hole_points, sketch_name):
    sketches = root_comp.sketches
    xyPlane = root_comp.xYConstructionPlane

    sketch = sketches.add(xyPlane)
    sketch.name = sketch_name

    points = sketch.sketchPoints
    created = adsk.core.ObjectCollection.create()

    for pot in hole_points:
        point = adsk.core.Point3D.create(pot["x"] / 10, pot["y"] / 10, panel_offset_z)
        created.add(points.add(point))

    return created


def run(context):
    ui = None
    hp_size = 8

    pots = [
        {
            "x": 20,
            "y": 20,
        },
    ]
    jacks = []
    sw = []
    leds = []

    try:
        app = adsk.core.Application.get()
        ui = app.userInterface
        design = app.activeProduct
        root_comp = design.rootComponent
        hole_distance = adsk.core.ValueInput.createByReal(-0.2)

        create_panel(root_comp, hp_size)
        add_mounting_holes(root_comp, hp_size)

        created_pots = add_points(root_comp, pots, 'pot')

        holes = root_comp.features.holeFeatures

        # Does not work, maybe we should try circles and just extrude them out?
        hole_input = holes.createSimpleInput(adsk.core.ValueInput.createByString('2 mm'))
        hole_input.setPositionBySketchPoints(created_pots)
        hole_input.setDistanceExtent(hole_distance)

        hole = holes.add(hole_input)

        created_jacks = add_points(root_comp, jacks, 'jack')
        created_switches = add_points(root_comp, sw, 'switch')
        created_leds = add_points(root_comp, leds, 'led')

    except:
        if ui:
            ui.messageBox('Failed:\n{}'.format(traceback.format_exc()))
