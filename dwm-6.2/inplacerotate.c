static void
insertclient(Client *item, Client *insertitem, int after)
{
	Client *c;

	if (item == NULL || insertitem == NULL || item == insertitem)
                return;

	detach(insertitem);
	if (!after && selmon->clients == item) {
		attach(insertitem);
		return;
	}
	if (after)
		c = item;
	else {
		for (c = selmon->clients; c; c = c->next)
                        if (c->next == item) break;
        }
	insertitem->next = c->next;
	c->next = insertitem;
}

static void
inplacerotate(const Arg *arg)
{
	int selidx = 0, i = 0;
	Client *c = NULL, *stail = NULL, *mhead = NULL, *mtail = NULL, *shead = NULL;

//	if(!selmon->sel || (selmon->sel->isfloating && !arg->f)) return;
	if(!selmon->sel || selmon->sel->isfloating)
                return;

	/* Determine positionings for insertclient */
	for (mhead = c = nexttiled(selmon->clients); c; c = nexttiled(c->next), i++) {
		if (selmon->sel == c)
                        selidx = i;
		if (i == selmon->nmaster - 1)
                        mtail = c;
		if (i == selmon->nmaster)
                        shead = c;
		stail = c;
	}

        switch (selmon->lt[selmon->sellt]->arrange == deck ? -arg->i : arg->i) {
                /* All clients rotate */
                case 1:
                        insertclient(stail, selmon->clients, 1);
                        break;
                case 2:
                        insertclient(selmon->clients, stail, 0);
                        break;
                /* Stack xor master rotate */
                case -1:
                        if (selidx >= selmon->nmaster)
                                insertclient(stail, shead, 1);
                        else
                                insertclient(mtail, mhead, 1);
                        break;
                case -2:
                        if (selidx >= selmon->nmaster)
                                insertclient(shead, stail, 0);
                        else
                                insertclient(mhead, mtail, 0);
                        break;
        }

	/* Restore focus position */
	for (i = 0, c = nexttiled(selmon->clients); c; c = nexttiled(c->next), i++) {
		if (i == selidx) {
                        focus(c);
                        break;
                }
	}
	arrange(selmon);
}
