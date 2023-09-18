#include <X11/Xlib.h>
#include <stdio.h>

Display *display;
Window window;
Atom CLIPBOARD, A, TARGETS, INCR, target;
XEvent event;
Atom actualTarget;
int format;
unsigned long bytesLeft, count;
unsigned char *data;

int main(const int argc, const char *const argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <target you want to request>\n", argv[0]);
        return 1;
    }
    display = XOpenDisplay(NULL);
    window = XCreateSimpleWindow(display, DefaultRootWindow(display), -10, -10, 1, 1, 0, 0, 0);
    CLIPBOARD = XInternAtom(display, "CLIPBOARD", False);
    A = XInternAtom(display, "A", False);
    INCR = XInternAtom(display, "INCR", False);
    target = XInternAtom(display, argv[1], False);

    XConvertSelection(display, CLIPBOARD, target, A, window, CurrentTime);

    do {
        XNextEvent(display, &event);
    } while (event.type != SelectionNotify || event.xselection.selection != CLIPBOARD);

    if (event.xselection.property == None) {
        fprintf(stderr, "Failed to convert selection to target %s\n", argv[1]);
        return 1;
    }

    XGetWindowProperty(display, window, A, 0, __LONG_MAX__ / 4, True, AnyPropertyType, &actualTarget, &format, &count,
                       &bytesLeft, &data);

    if (actualTarget != INCR) {
        fwrite(data, 1, format * count / 8, stdout);
        return 0;
    }

    XSelectInput(display, window, PropertyChangeMask);
    do {
        do {
            XNextEvent(display, &event);
        } while (event.type != PropertyNotify || event.xproperty.state != PropertyNewValue);

        XGetWindowProperty(display, window, A, 0, __LONG_MAX__ / 4, True, AnyPropertyType, &actualTarget, &format,
                           &count, &bytesLeft, &data);
        fwrite(data, 1, format * count / 8, stdout);
    } while (count > 0);

    return 0;
}