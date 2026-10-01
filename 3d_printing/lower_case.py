# lower_case.py
# This builds the lower case (there is no upper case). The lower case
# is the shell in which everything in the alethiometer lives
#
# =====================================================================
# HOW IT IS BUILT (the same steps you would click by hand)
# =====================================================================
#   1. Sketch the outside octagon on the XY plane.
#   2. Extrude it 60 mm up. That gives a solid octagonal "puck".
#   3. Shell the puck 2.5 mm, removing the top face. Shell hollows a solid
#      and leaves walls of the given thickness, so one step makes both the
#      walls and the floor.
#   4. Cut each opening: draw its rectangle on an upright plane through
#      the middle of the box, then cut from there towards its wall and
#      1 mm past it. One small helper, cut_window(), does all five.
#
# WHY IS THIS MORE CODE THAN A FEW CLICKS?
#   In Fusion's API every feature takes three moves: make an "input"
#   object, fill in its settings, then add it. Clicking does all three at
#   once. The API also works in centimetres, so every millimetre value is
#   divided by 10 before it goes in. Everything else here is comments.
#
# =====================================================================
# HOW TO RUN IN AUTODESK FUSION
# =====================================================================
#   1. Open the design that holds the assembly, or create a new Hybrid Design. 
#       If it already has a "Lower Case" component from an earlier run, delete 
#       it first: the script always adds a new one and does not check.
#   2. Utilities tab -> Scripts and Add-Ins, then run this script.
#   3. A short message pops up when it is done.
#
# TO CHANGE THE BOX
#   Edit the numbers in the PARAMETERS block and run it again. Keep every
#   opening narrower than the flat part of its wall (about 91 mm at the
#   default sizes) and below the rim. If you move CABLE_WALL or TOUCH_WALL,
#   make the same change in motor_plate_stand.py.

import adsk.core
import adsk.fusion
import traceback
import math


# =====================================================================
# PARAMETERS (all in millimetres)
# =====================================================================

# The cover octagon is the top cover / inlay outline the box wraps around.
# The box is built outward from it, so these three set the box's shape.
COVER_SIZE_MM = 104.0     # cover width across the flats
COVER_CHAMFER_MM = 7.2    # how far each corner is cut back
CLEARANCE_MM = 1.95       # gap between the cover and the inside wall

WALL_MM = 2.5             # wall AND floor thickness (one shell does both)
HEIGHT_MM = 60.0          # overall height, case underside to rim

# What stands inside the case, used to place the thumbwheel cutouts.
SUPPORT_WALL_H_MM = 30.0                      # motor_plate_stand.py
GEAR_FLOOR_ABOVE_PLATE_UNDERSIDE_MM = 10.5    # motor_plate.py: 3.0 + 7.5
GEAR_FLOOR_Z_MM = (WALL_MM + SUPPORT_WALL_H_MM
                   + GEAR_FLOOR_ABOVE_PLATE_UNDERSIDE_MM)          # 43.0

# Which walls the two electronics openings go on. The touch window sits on
# the -Y wall, below the S2 thumbwheel (it was on -X, moved one wall
# anticlockwise to be easier to reach). The cable window is on +X, the only
# wall with no thumbwheel. The same two
# values live in motor_plate_stand.py.
CABLE_WALL = '+X'
TOUCH_WALL = '-Y'

# Cable window. Both openings are set the same way: a size, plus the
# height of the bottom edge measured from the case underside. To move
# either one up or down, change its BOTTOM_Z and nothing else, here and in
# motor_plate_stand.py, which carries matching openings.
#
# The cable window used to sit on the floor top at 2.5. After the first
# print it was raised 13.0 mm to 15.5 and made 0.2 mm taller. After a
# second fit it came down 3.0 mm to 12.5 and gained another 1.0 mm of
# height, since the window was sitting almost too high for the USB lead.
CABLE_WIDTH_MM = 14.0
CABLE_HEIGHT_MM = 8.2
CABLE_BOTTOM_Z_MM = 12.5

# Touch window: must line up with the stand's, so these same three numbers
# appear in motor_plate_stand.py.
TOUCH_WIDTH_MM = 20.0
TOUCH_HEIGHT_MM = 13.0
TOUCH_BOTTOM_Z_MM = 9.0

# Thumbwheel openings: from the gear floor up this far.
THUMBWHEEL_HEIGHT_MM = 6.0

# The three side wheels, copied from motor_plate.py's AXLE_HOLES_RAW with
# the Myth Made shaft position (58.798, 56.606) already subtracted, so
# these are in our shaft-at-origin frame. Only these three wells reach
# past the case wall, which is how we know they are the thumbwheels.
#   (name, wall, wheel centre along the wall, wheel centre distance from
#    the shaft, gear well radius)
# "Along the wall" is x for the Y walls and y for the X walls.
THUMBWHEELS = [
    ('N1', '+Y', -0.242, 48.689, 12.87),
    ('E1', '-X', +0.074, 47.926, 13.23),
    ('S2', '-Y', -0.322, 48.909, 14.16),
]


# =====================================================================
# HELPERS
# =====================================================================

def mm(value_mm):
    """
    Convert millimetres to centimetres.

    Fusion's screen shows millimetres, but its API stores every length in
    centimetres. So 60 mm has to be passed in as 6.0. Every coordinate in
    this script goes through this helper on its way into Fusion.
    """
    return value_mm / 10.0


def mm_value(value_mm):
    """A distance for a feature, written as text such as '60.0 mm' so it
    reads naturally in the timeline if you edit the feature later."""
    return adsk.core.ValueInput.createByString('{} mm'.format(value_mm))


def octagon_corners(offset_mm):
    """
    The eight corners (x, y) of the cover octagon, grown outward by
    offset_mm, centred on the origin.

    The octagon is a square with its four corners cut off at 45 degrees.
    Two numbers describe it:
        half  = half the width across the flats
        cut   = how far each corner is cut back along the edge

    Growing the whole shape outward by a distance d:
        - every flat side moves out by d, so half grows by d
        - each 45 degree corner edge also moves out by d, and a bit of
          trigonometry shows the corner cut grows by d * (2 - sqrt(2)),
          about 0.586 * d

    The script only draws the OUTSIDE octagon (cover + clearance + wall);
    the shell makes the inside one.
    """
    half = COVER_SIZE_MM / 2.0 + offset_mm
    cut = COVER_CHAMFER_MM + offset_mm * (2.0 - math.sqrt(2.0))
    a = half - cut      # where each flat side ends and a corner cut begins

    # Walk around the octagon anticlockwise, two corners per flat side.
    return [
        (a, half), (-a, half),        # +Y flat
        (-half, a), (-half, -a),      # -X flat
        (-a, -half), (a, -half),      # -Y flat
        (half, -a), (half, a),        # +X flat
    ]


def draw_closed_loop(sketch, corners_mm):
    """
    Draw straight lines from corner to corner, and back to the start, so
    the shape closes and Fusion can turn it into a profile.

    corners_mm are MODEL coordinates (x, y, z) in millimetres, i.e. the
    positions in the design itself. modelToSketchSpace converts each one
    into the sketch's own flat coordinates. That matters because upright
    sketch planes have their own axes pointing different ways from the
    model's, and this conversion takes care of that so we can always
    think in model coordinates.
    """
    points = [sketch.modelToSketchSpace(
                  adsk.core.Point3D.create(mm(x), mm(y), mm(z)))
              for (x, y, z) in corners_mm]

    # Line from each corner to the next. The "% count" wraps the last
    # corner back round to the first, which closes the loop.
    lines = sketch.sketchCurves.sketchLines
    count = len(points)
    for i in range(count):
        lines.addByTwoPoints(points[i], points[(i + 1) % count])


def direction_along(sketch, axis, sign=+1):
    """
    Which way to extrude so the feature grows along model axis 'x', 'y'
    or 'z', towards + (sign = +1) or - (sign = -1).

    An extrude goes either along the SKETCH's normal (Positive) or against
    it (Negative). That is not always the way the construction plane's
    own normal points, which is what put the stand's touch window on the
    wrong wall once. So we ask the sketch itself: convert the sketch point
    (0, 0, 1), one step along its normal, into model coordinates and see
    which way it moved.
    """
    start = sketch.sketchToModelSpace(adsk.core.Point3D.create(0, 0, 0))
    step = sketch.sketchToModelSpace(adsk.core.Point3D.create(0, 0, 1))
    moved = {'x': step.x - start.x,
             'y': step.y - start.y,
             'z': step.z - start.z}[axis]

    if (moved > 0) == (sign > 0):
        return adsk.fusion.ExtentDirections.PositiveExtentDirection
    return adsk.fusion.ExtentDirections.NegativeExtentDirection


def cut_window(comp, body, name, wall, centre, width, bottom, height):
    """
    Cut a rectangular opening through one wall of the box.

        wall     which wall: '+X', '-X', '+Y' or '-Y'
        centre   where the opening's middle sits ALONG that wall
                 (x for the Y walls, y for the X walls), in mm
        width    opening width along the wall, in mm
        bottom   height of the opening's bottom edge above the case
                 underside (Z), in mm
        height   opening height, in mm

    How it works: draw the rectangle on the upright plane through the
    middle of the box that faces that wall (XZ for the Y walls, YZ for
    the X walls), then cut from there towards the wall. Starting from the
    middle, the only solid in the rectangle's path is that one wall.

    The cut is a fixed distance, from the middle to 1 mm past the outside
    face, rather than "through all". Fusion occasionally skipped a
    through-all cut on this shelled body; a finite cut that clearly exits
    the wall has never missed.
    """
    axis = wall[1].lower()          # 'x' or 'y': the axis the wall faces
    sign = +1 if wall[0] == '+' else -1

    left = centre - width / 2.0
    right = centre + width / 2.0
    top = bottom + height

    if axis == 'y':
        # Y walls: rectangle on the XZ plane (y = 0), spread along x.
        plane = comp.xZConstructionPlane
        corners = [(left, 0.0, bottom), (right, 0.0, bottom),
                   (right, 0.0, top), (left, 0.0, top)]
    else:
        # X walls: rectangle on the YZ plane (x = 0), spread along y.
        plane = comp.yZConstructionPlane
        corners = [(0.0, left, bottom), (0.0, right, bottom),
                   (0.0, right, top), (0.0, left, top)]

    sketch = comp.sketches.add(plane)
    sketch.name = name
    draw_closed_loop(sketch, corners)

    # From the centre plane to the outside face is half the outside size.
    outside_half = COVER_SIZE_MM / 2.0 + CLEARANCE_MM + WALL_MM   # 56.45
    extrudes = comp.features.extrudeFeatures
    cut_input = extrudes.createInput(
        sketch.profiles.item(0),
        adsk.fusion.FeatureOperations.CutFeatureOperation)
    cut_input.setOneSideExtent(
        adsk.fusion.DistanceExtentDefinition.create(
            mm_value(outside_half + 1.0)),
        direction_along(sketch, axis, sign))
    cut_input.participantBodies = [body]   # only cut the box
    extrudes.add(cut_input).name = name + ' cut'


def thumbwheel_cutout_width(wheel_distance, well_radius):
    """
    How wide a thumbwheel cutout must be for the wheel to pass through.

    The wheel pokes out through the wall, so the opening must be as wide
    as the wheel where it meets the wall. A straight cutout is tightest at
    the INSIDE face, where the wheel is widest, so we measure there. The
    gear WELL's radius stands in for the wheel's size, because the Myth
    Made well is the wheel plus its running clearance.

    Picture the wheel as a circle and the inside wall face as a straight
    line crossing it. The line is (inside half-width - wheel distance)
    away from the wheel's centre, and Pythagoras gives half the chord.
    """
    inside_half = COVER_SIZE_MM / 2.0 + CLEARANCE_MM     # 53.95
    gap = inside_half - wheel_distance
    return 2.0 * math.sqrt(well_radius ** 2 - gap ** 2)


# =====================================================================
# THE BUILD
# =====================================================================

def run(context):
    ui = None
    try:
        app = adsk.core.Application.get()
        ui = app.userInterface
        design = adsk.fusion.Design.cast(app.activeProduct)

        # A new component at the assembly origin (an identity transform:
        # no shift, no turn). Everything below is built inside it, so the
        # case, the motor plate and its stand can share one design.
        occurrence = design.rootComponent.occurrences.addNewComponent(
            adsk.core.Matrix3D.create())
        comp = occurrence.component
        comp.name = 'Lower Case'
        extrudes = comp.features.extrudeFeatures

        # -------------------------------------------------------------
        # STEP 1: sketch the outside octagon on the XY plane (Z = 0)
        # -------------------------------------------------------------
        # The outside face of the wall is the cover octagon grown by the
        # clearance plus the wall thickness. z is 0 for every corner
        # because the octagon lies flat on the XY plane.
        outline = comp.sketches.add(comp.xYConstructionPlane)
        outline.name = 'Case outline (outside octagon)'
        draw_closed_loop(outline, [
            (x, y, 0.0) for (x, y) in octagon_corners(CLEARANCE_MM + WALL_MM)])

        # -------------------------------------------------------------
        # STEP 2: extrude the octagon up into a solid puck
        # -------------------------------------------------------------
        # createInput -> fill in settings -> add: the three API moves.
        # NewBodyFeatureOperation means "make a new body", rather than
        # join onto or cut from an existing one.
        puck_input = extrudes.createInput(
            outline.profiles.item(0),
            adsk.fusion.FeatureOperations.NewBodyFeatureOperation)
        puck_input.setOneSideExtent(
            adsk.fusion.DistanceExtentDefinition.create(mm_value(HEIGHT_MM)),
            direction_along(outline, 'z'))
        puck = extrudes.add(puck_input)
        puck.name = 'Case blank'
        puck.bodies.item(0).name = 'Lower Case body'

        # -------------------------------------------------------------
        # STEP 3: shell the puck to leave walls and a floor
        # -------------------------------------------------------------
        # The face to remove is the top of the puck. An extrude remembers
        # the face it finished on ("end face"), so we grab it directly.
        # insideThickness makes the wall grow inward from the outside
        # faces, so the outside size stays exactly as sketched.
        faces_to_remove = adsk.core.ObjectCollection.create()
        faces_to_remove.add(puck.endFaces.item(0))
        shells = comp.features.shellFeatures
        shell_input = shells.createInput(faces_to_remove, False)
        shell_input.insideThickness = mm_value(WALL_MM)
        shell = shells.add(shell_input)
        shell.name = 'Shell (wall and floor)'

        # Cuts change this body in place, so one reference serves for all
        # five openings below.
        body = shell.bodies.item(0)

        # -------------------------------------------------------------
        # STEP 4: cut the openings
        # -------------------------------------------------------------
        # Cable window: centred, CABLE_BOTTOM_Z_MM up the wall.
        cut_window(comp, body, 'Cable window', CABLE_WALL,
                   0.0, CABLE_WIDTH_MM, CABLE_BOTTOM_Z_MM, CABLE_HEIGHT_MM)

        # Touch window: centred, below the top of the stand.
        cut_window(comp, body, 'Touch sensor window', TOUCH_WALL,
                   0.0, TOUCH_WIDTH_MM, TOUCH_BOTTOM_Z_MM, TOUCH_HEIGHT_MM)

        # Thumbwheels: one opening per side wheel, centred on its wheel,
        # from the gear floor up.
        for (name, wall, along, distance, well_r) in THUMBWHEELS:
            cut_window(comp, body, 'Thumbwheel ' + name, wall, along,
                       thumbwheel_cutout_width(distance, well_r),
                       GEAR_FLOOR_Z_MM, THUMBWHEEL_HEIGHT_MM)

        app.activeViewport.fit()
        ui.messageBox('"Lower Case" built.')

    except:
        if ui:
            ui.messageBox('Lower Case build failed:\n{}'.format(
                traceback.format_exc()))

