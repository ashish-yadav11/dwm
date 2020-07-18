void
moveresize(const Arg *arg) {
	/* only floating windows can be moved */
	int x, y, w, h, nx, ny, nw, nh, ox, oy, ow, oh;
        int ptrx, ptry;

	if (!selmon->sel || !arg)
		return;
	if (selmon->lt[selmon->sellt]->arrange && !selmon->sel->isfloating)
		return;
	if (sscanf((char *)arg->v, "%d %d %d %d", &x, &y, &w, &h) != 4)
		return;

	/* compute new window position
         * prevent window from being positioned outside the current monitor on first attempt */
        if (w) {
                int vselcx = selmon->sel->x + 2 * selmon->sel->bw;
                int vselmw = selmon->wx + selmon->ww;

                nw = selmon->sel->w + w;
                if (vselcx + selmon->sel->w < vselmw && vselcx + nw > vselmw)
                        nw = vselmw - vselcx;
        } else
                nw = selmon->sel->w;

        if (h) {
                int vselcy = selmon->sel->y + 2 * selmon->sel->bw;
                int vselmh = selmon->wy + selmon->wh;

                nh = selmon->sel->h + h;
                if (vselcy + selmon->sel->h < vselmh && vselcy + nh > vselmh)
                        nh = vselmh - vselcy;
        } else
                nh = selmon->sel->h;

        if (x) {
                int vselcw = WIDTH(selmon->sel);
                int vselmw = selmon->wx + selmon->ww;

                nx = selmon->sel->x + x;
                if (selmon->sel->x > selmon->wx && nx < selmon->wx)
                        nx = selmon->wx;
                else if (selmon->sel->x + vselcw < vselmw && nx + vselcw > vselmw)
                        nx = vselmw - vselcw;
        } else
                nx = selmon->sel->x;

        if (y) {
                int vselch = HEIGHT(selmon->sel);
                int vselmh = selmon->wy + selmon->wh;

                ny = selmon->sel->y + y;
                if (selmon->sel->y > selmon->wy && ny < selmon->wy)
                        ny = selmon->wy;
                else if (selmon->sel->y + vselch < vselmh && ny + vselch > vselmh)
                        ny = vselmh - vselch;
        } else
                ny = selmon->sel->y;

	ox = selmon->sel->x;
	oy = selmon->sel->y;
	ow = selmon->sel->w;
	oh = selmon->sel->h;

	XRaiseWindow(dpy, selmon->sel->win);
	Bool xqp = getrootptr(&ptrx, &ptry);
	resize(selmon->sel, nx, ny, nw, nh, True);

	/* move cursor along with the window to avoid problems caused by the sloppy focus */
	if (xqp && ox <= ptrx && (ox + ow) >= ptrx && oy <= ptry && (oy + oh) >= ptry)
		XWarpPointer(dpy, None, None, 0, 0, 0, 0,
                             selmon->sel->x - ox + selmon->sel->w - ow,
                             selmon->sel->y - oy + selmon->sel->h - oh);
}
