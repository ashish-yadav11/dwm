/* the following two functions assume non-NULL and distinct c and p */
static void
moveafter(Client *c, Client *p)
{
        detach(c);
        c->next = p->next;
        p->next = c;
}

static void
movebefore(Client *c, Client *p)
{
        Client *i;

        detach(c);
        if (p == selmon->clients)
                attach(c);
        else {
                for (i = selmon->clients; i->next != p; i = i->next);
                i->next = c;
                c->next = p;
        }
}

static void
inplacerotate(const Arg *arg)
{
        int i, loc;
        Client *c, *head, *tail;

        if (!selmon->sel || selmon->sel->isfloating
                         || !selmon->lt[selmon->sellt]->arrange)
                return;
        /* all clients rotate */
        if (abs(arg->i) == 1 || selmon->nmaster == 0) {
                c = head = nexttiled(selmon->clients);
                do tail = c; while ((c = nexttiled(c->next)));
        } else {
                for (loc = 0, c = selmon->clients; c != selmon->sel; c = c->next)
                        if (!c->isfloating && ISVISIBLE(c))
                                loc++;
                /* master clients rotate */
                if (loc < selmon->nmaster) {
                        c = head = nexttiled(selmon->clients);
                        i = selmon->nmaster;
                        do tail = c; while (--i > 0 && (c = nexttiled(c->next)));
                /* stack clients rotate */
                } else {
                        for (i = selmon->nmaster, c = selmon->clients;
                             c->isfloating || !ISVISIBLE(c) || i-- > 0;
                             c = c->next);
                        head = c;
                        do tail = c; while ((c = nexttiled(c->next)));
                }
        }
        if (head == tail)
                return;
        if (arg->i < 0)
                moveafter(head, tail);
        else
                movebefore(tail, head);
        arrange(selmon);
}

static void
inplacezoom(const Arg *arg)
{
        int i, loc;
        Client *c, *head, *tail, *mtail;

        if (!selmon->sel || selmon->sel->isfloating
                         || !selmon->lt[selmon->sellt]->arrange)
                return;
        /* make master of all clients */
        if (arg->i >= 0 || selmon->nmaster == 0) {
                c = head = nexttiled(selmon->clients);
                do tail = c; while ((c = nexttiled(c->next)));
                while (head != selmon->sel) {
                        moveafter(head, tail);
                        tail = head;
                        head = nexttiled(selmon->clients);
                }
        } else {
                for (loc = 0, c = selmon->clients; c != selmon->sel; c = c->next)
                        if (!c->isfloating && ISVISIBLE(c))
                                loc++;
                /* make master of master clients */
                if (loc < selmon->nmaster) {
                        c = head = nexttiled(selmon->clients);
                        i = selmon->nmaster;
                        do tail = c; while (--i > 0 && (c = nexttiled(c->next)));
                        while (head != selmon->sel) {
                                moveafter(head, tail);
                                tail = head;
                                head = nexttiled(selmon->clients);
                        }
                /* make master of stack rotate */
                } else {
                        for (i = selmon->nmaster - 1, c = selmon->clients;
                             c->isfloating || !ISVISIBLE(c) || i-- > 0;
                             c = c->next);
                        mtail = c;
                        head = nexttiled(mtail->next);
                        do tail = c; while ((c = nexttiled(c->next)));
                        while (head != selmon->sel) {
                                moveafter(head, tail);
                                tail = head;
                                head = nexttiled(mtail->next);
                        }
                }
        }
        arrange(selmon);
}
