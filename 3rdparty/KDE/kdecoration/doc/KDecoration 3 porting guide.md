# Renames
The `KDecoration2` library and namespace were renamed to `KDecoration3`.

`DecoratedClient` was renamed to `DecoratedWindow`

`Decoration::client()` was renamed to `Decoration::window()`

# Fractional scaling
All geometry has been switched from integer logical units to floating point logical units, which are snapped to the pixel grid.
The pixel grid is defined by the scale factor of the window — in logical coordinates, each pixel is `1.0 / window()->scale()` large.

A naive port that should make things work the same as before is to just round all geometry from the API to integers. However, to make fractional scaling work well, the decoration must snap all geometry values to the pixel grid.
Currently, that only affects the border sizes, which you can snap to the pixel grid with `KDecoration3::snapToPixelGrid(value, scale)` from the `KDecoration3/scalehelpers.h` header. Note that this *may* end up snapping the value towards zero with scale factors below 100%; if you want to prevent that, take the maximum of that and the size of a single pixel: `std::max(KDecoration3::snapToPixelGrid(value, scale), KDecoration3::pixelSize(scale))`.

The fractional parts of geometry and the scale factor have to be taken into account when painting as well.
QPainter automatically applies scaling for you on its own, but it doesn't do a great job at it. To make it paint without blurriness or cut off parts, you need to:
1. avoid integer versions of QPainter methods. More specifically, watch out for `QPainter::setClipRect(int, int, int, int)`, as it silently clips floating point numbers. Instead, use `QPainter::setClipRect(QRectF(...))`
2. adjust the size and position of lines to be snapped to the pixel grid

For example, to paint a 1px wide outline for the title bar, you can't just do
```
painter->setClipRect(titleBarRect.x(), titleBarRect.y(), titleBarRect.width(), titleBarRect.height(), Qt::IntersectClip)
painter->setBrush(QPen(QColor::red, 1));
painter->paintRect(rect());
```
because QPainter paints the line *around* the window rather than in the outermost pixels, and applies anti-aliasing on top of that. Instead, do
```
painter->setClipRect(titleBarRect, Qt::IntersectClip);
const qreal width = KDecoration3::pixelSize(window()->scale());
painter->setBrush(QPen(QColor::red, width));
painter->paintRect(rect().adjusted(width / 2, width / 2, -width / 2, -width / 2));
```

# Reacting to scale changes
When the `nextScale` of the decorated window changes, the decoration needs to update the border sizes with values that are snapped to the new scale, or there may be gaps between the window content and the decoration.
