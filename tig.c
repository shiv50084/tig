 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */
static int read_properties(const char *cmd, int separator, int (*read)(char *, int, char *, int));
static size_t utf8_length(const char *string, size_t max_width, int *coloffset, int *trimmed);
#define TIG_HELP	"(d)iff, (l)og, (m)ain, (q)uit, (h)elp"
#define AUTHOR_COLS	20

	REQ_VIEW_CLOSE,
	REQ_NEXT,
	REQ_PREVIOUS,
static struct ref **get_refs(char *id);
#define BUFPUT(x) do { if (bufsize < SIZEOF_CMD) buf[bufsize++] = (x); } while (0)
static const char usage[] =
VERSION " (" __DATE__ ")\n"
"\n"
"Usage: tig [options]\n"
"   or: tig [options] [--] [git log options]\n"
"   or: tig [options] log  [git log options]\n"
"   or: tig [options] diff [git diff options]\n"
"   or: tig [options] show [git show options]\n"
"   or: tig [options] <    [git command output]\n"
"\n"
"Options:\n"
"  -l                          Start up in log view\n"
"  -d                          Start up in diff view\n"
"  -n[I], --line-number[=I]    Show line numbers with given interval\n"
"  -t[N], --tab-size[=N]       Set number of spaces for tab expansion\n"
"  --                          Mark end of tig options\n"
"  -v, --version               Show version and exit\n"
"  -h, --help                  Show help message and exit\n";

static char opt_encoding[20]	= "";
static bool opt_utf8		= TRUE;
		/**
		 * -h, --help::
		 *	Show help message and exit.
		 **/
		if (!strcmp(opt, "-h") ||
		    !strcmp(opt, "--help")) {
			printf(usage);
			return FALSE;
		}

	if (*opt_encoding && strcasecmp(opt_encoding, "UTF-8"))
		opt_utf8 = FALSE;

/**
 * ENVIRONMENT VARIABLES
 * ---------------------
 * Several options related to the interface with git can be configured
 * via environment options.
 *
 * Repository references
 * ~~~~~~~~~~~~~~~~~~~~~
 * Commits that are referenced by tags and branch heads will be marked
 * by the reference name surrounded by '[' and ']':
 *
 *	2006-03-26 19:42 Petr Baudis         | [cogito-0.17.1] Cogito 0.17.1
 *
 * If you want to filter out certain directories under `.git/refs/`, say
 * `tmp` you can do it by setting the following variable:
 *
 *	$ TIG_LS_REMOTE="git ls-remote . | sed /\/tmp\//d" tig
 *
 * Or set the variable permanently in your environment.
 *
 * TIG_LS_REMOTE::
 *	Set command for retrieving all repository references. The command
 *	should output data in the same format as git-ls-remote(1).
 **/

#define TIG_LS_REMOTE \
	"git ls-remote . 2>/dev/null"

/**
 * [[history-commands]]
 * History commands
 * ~~~~~~~~~~~~~~~~
 * It is possible to alter which commands are used for the different views.
 * If for example you prefer commits in the main view to be sorted by date
 * and only show 500 commits, use:
 *
 *	$ TIG_MAIN_CMD="git log --date-order -n500 --pretty=raw %s" tig
 *
 * Or set the variable permanently in your environment.
 *
 * Notice, how `%s` is used to specify the commit reference. There can
 * be a maximum of 5 `%s` ref specifications.
 *
 * TIG_DIFF_CMD::
 *	The command used for the diff view. By default, git show is used
 *	as a backend.
 *
 * TIG_LOG_CMD::
 *	The command used for the log view. If you prefer to have both
 *	author and committer shown in the log view be sure to pass
 *	`--pretty=fuller` to git log.
 *
 * TIG_MAIN_CMD::
 *	The command used for the main view. Note, you must always specify
 *	the option: `--pretty=raw` since the main view parser expects to
 *	read that format.
 **/

#define TIG_DIFF_CMD \
	"git show --patch-with-stat --find-copies-harder -B -C %s"

#define TIG_LOG_CMD	\
	"git log --cc --stat -n100 %s"

#define TIG_MAIN_CMD \
	"git log --topo-order --stat --pretty=raw %s"

/* ... silently ignore that the following are also exported. */

#define TIG_HELP_CMD \
	"man tig 2>/dev/null"

#define TIG_PAGER_CMD \
	""


struct line {
	enum line_type type;
	void *data;		/* User data */
};
 * The viewer
 * ----------
 * The display consists of a status window on the last line of the screen and
 * one or more views. The default is to only show one view at the time but it
 * is possible to split both the main and log view to also show the commit
 * diff.
 * If you are in the log view and press 'Enter' when the current line is a
 * commit line, such as:
 *	commit 4d55caff4cc89335192f3e566004b4ceef572521
 * You will split the view so that the log view is displayed in the top window
 * and the diff view in the bottom window. You can switch between the two
 * views by pressing 'Tab'. To maximize the log view again, simply press 'l'.
struct view;
struct view_ops;
/* The display array of active views and the index of the current view. */
static struct view *display[2];
static unsigned int current_view;
#define foreach_view(view, i) \
	for (i = 0; i < ARRAY_SIZE(display) && (view = display[i]); i++)
#define displayed_views()	(display[1] != NULL ? 2 : 1)
 * Current head and commit ID
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~
 * The viewer keeps track of both what head and commit ID you are currently
 * viewing. The commit ID will follow the cursor line and change everytime time
 * you highlight a different commit. Whenever you reopen the diff view it
 * will be reloaded, if the commit ID changed.
 * The head ID is used when opening the main and log view to indicate from
 * what revision to show history.
static char ref_commit[SIZEOF_REF]	= "HEAD";
static char ref_head[SIZEOF_REF]	= "HEAD";


	struct view_ops *ops;	/* View operations */
	/* If non-NULL, points to the view that opened this view. If this view
	 * is closed tig will switch back to the parent view. */
	struct view *parent;

	struct line *line;	/* Line index */
struct view_ops {
	/* What type of content being displayed. Used in the title bar. */
	const char *type;
	/* Draw one line; @lineno must be < view->height. */
	bool (*draw)(struct view *view, struct line *line, unsigned int lineno);
	/* Read one line; updates view->line. */
	bool (*read)(struct view *view, struct line *prev, char *data);
	/* Depending on view, change display based on current line. */
	bool (*enter)(struct view *view, struct line *line);
};

#define VIEW_STR(name, cmd, env, ref, ops) \
	{ name, cmd, #env, ref, ops }
#define VIEW_(id, name, ops, ref) \
	VIEW_STR(name, TIG_##id##_CMD,  TIG_##id##_CMD, ref, ops)
/**
 * Views
 * ~~~~~
 * tig(1) presents various 'views' of a repository. Each view is based on output
 * from an external command, most often 'git log', 'git diff', or 'git show'.
 *
 * The main view::
 *	Is the default view, and it shows a one line summary of each commit
 *	in the chosen list of revisions. The summary includes commit date,
 *	author, and the first line of the log message. Additionally, any
 *	repository references, such as tags, will be shown.
 *
 * The log view::
 *	Presents a more rich view of the revision log showing the whole log
 *	message and the diffstat.
 *
 * The diff view::
 *	Shows either the diff of the current working tree, that is, what
 *	has changed since the last commit, or the commit diff complete
 *	with log message, diffstat and diff.
 *
 * The pager view::
 *	Is used for displaying both input from stdin and output from git
 *	commands entered in the internal prompt.
 *
 * The help view::
 *	Displays the information from the tig(1) man page. For the help view
 *	to work you need to have the tig(1) man page installed.
 **/
	VIEW_(MAIN,  "main",  &main_ops,  ref_head),
	VIEW_(DIFF,  "diff",  &pager_ops, ref_commit),
	VIEW_(LOG,   "log",   &pager_ops, ref_head),
	VIEW_(HELP,  "help",  &pager_ops, "static"),
	VIEW_(PAGER, "pager", &pager_ops, "static"),
static bool
draw_view_line(struct view *view, unsigned int lineno)
{
	if (view->offset + lineno >= view->lines)
		return FALSE;
	return view->ops->draw(view, &view->line[view->offset + lineno], lineno);
}
		if (!draw_view_line(view, lineno))

/**
 * Title windows
 * ~~~~~~~~~~~~~
 * Each view has a title window which shows the name of the view, current
 * commit ID if available, and where the view is positioned:
 *
 *	[main] c622eefaa485995320bc743431bae0d497b1d875 - commit 1 of 61 (1%)
 *
 * By default, the title of the current view is highlighted using bold font.
 **/

static void
update_view_title(struct view *view)
{
	if (view == display[current_view])
		wbkgdset(view->title, get_line_attr(LINE_TITLE_FOCUS));
	else
		wbkgdset(view->title, get_line_attr(LINE_TITLE_BLUR));

	werase(view->title);
	wmove(view->title, 0, 0);

	if (*view->ref)
		wprintw(view->title, "[%s] %s", view->name, view->ref);
	else
		wprintw(view->title, "[%s]", view->name);

	if (view->lines || view->pipe) {
		unsigned int lines = view->lines
				   ? (view->lineno + 1) * 100 / view->lines
				   : 0;

		wprintw(view->title, " - %s %d of %d (%d%%)",
			view->ops->type,
			view->lineno + 1,
			view->lines,
			lines);
	}

	if (view->pipe) {
		time_t secs = time(NULL) - view->start_time;

		/* Three git seconds are a long time ... */
		if (secs > 2)
			wprintw(view->title, " %lds", secs);
	}

	wmove(view->title, 0, view->width - 1);
	wrefresh(view->title);
}

			view->win = newwin(view->height, 0, offset, 0);
			wresize(view->win, view->height, view->width);
redraw_display(void)
	struct view *view;
	int i;
	foreach_view (view, i) {
		redraw_view(view);
		update_view_title(view);
	}
}
static void
update_display_cursor(void)
{
	struct view *view = display[current_view];
	/* Move the cursor to the right-most column of the cursor line.
	 *
	 * XXX: This could turn out to be a bit expensive, but it ensures that
	 * the cursor does not jump around. */
		wmove(view->win, view->lineno - view->offset, view->width - 1);
		wrefresh(view->win);
do_scroll_view(struct view *view, int lines, bool redraw)
			if (!draw_view_line(view, line))
		draw_view_line(view, 0);
		draw_view_line(view, view->lineno - view->offset);
	if (!redraw)
		return;

	do_scroll_view(view, lines, TRUE);
move_view(struct view *view, enum request request, bool redraw)
		draw_view_line(view,  prev_lineno);
		do_scroll_view(view, steps, redraw);
	draw_view_line(view, view->lineno - view->offset);

	if (!redraw)
		return;
static void
end_update(struct view *view)
{
	if (!view->pipe)
		return;
	set_nonblocking_input(FALSE);
	if (view->pipe == stdin)
		fclose(view->pipe);
	else
		pclose(view->pipe);
	view->pipe = NULL;
}

	if (view->pipe)
		end_update(view);

			if (view->line[i].data)
				free(view->line[i].data);
	struct line *tmp;
		struct line *prev = view->lines
				  ? &view->line[view->lines - 1]
				  : NULL;

		if (!view->ops->read(view, prev, line))
		report("");
	int nviews = displayed_views();
	struct view *base_view = display[0];
	/* Resize the view when switching between split- and full-screen,
	 * or when switching between two different full-screen views. */
	if (nviews != displayed_views() ||
	    (nviews == 1 && base_view != display[0]))
		resize_display();
		do_scroll_view(prev, lines, TRUE);
		if (split && !backgrounded) {
			/* "Blur" the previous view. */
		}
		view->parent = prev;
	if (view->pipe && view->lines == 0) {
		report("");
		move_view(view, request, TRUE);
	case REQ_NEXT:
	case REQ_PREVIOUS:
		request = request == REQ_NEXT ? REQ_MOVE_DOWN : REQ_MOVE_UP;

		if (view == VIEW(REQ_VIEW_DIFF) &&
		    view->parent == VIEW(REQ_VIEW_MAIN)) {
			bool redraw = display[1] == view;

			view = view->parent;
			move_view(view, request, redraw);
			if (redraw)
				update_view_title(view);
		} else {
			move_view(view, request, TRUE);
			break;
		}
		return view->ops->enter(view, &view->line[view->lineno]);
		int nviews = displayed_views();
		redraw_display();
		report("%s (built %s)", VERSION, __DATE__);
		redraw_display();
	case REQ_VIEW_CLOSE:
		if (view->parent) {
			memset(display, 0, sizeof(display));
			current_view = 0;
			display[current_view] = view->parent;
			view->parent = NULL;
			resize_display();
			redraw_display();
			break;
		}
		/* Fall-through */
 * Pager backend
pager_draw(struct view *view, struct line *line, unsigned int lineno)
	char *text = line->data;
	enum line_type type = line->type;
	int textlen = strlen(text);
			string_copy(view->ref, text + 7);
		while (text && col_offset + col < view->width) {
			char *pos = text;
			if (*text == '\t') {
				text++;
				pos = spaces;
				text = strchr(text, '\t');
				cols = line ? text - pos : strlen(pos);
			waddnstr(view->win, pos, MIN(cols, cols_max));
		for (; pos < textlen && col < view->width; pos++, col++)
			if (text[pos] == '\t')
		waddnstr(view->win, text, pos);
pager_read(struct view *view, struct line *prev, char *line)
	    !*line && prev && !*((char *) prev->data))
	view->line[view->lines].data = strdup(line);
	if (!view->line[view->lines].data)
	view->line[view->lines].type = get_line_type(line);

pager_enter(struct view *view, struct line *line)
	int split = 0;
	if (line->type == LINE_COMMIT &&
	   (view == VIEW(REQ_VIEW_LOG) ||
	    view == VIEW(REQ_VIEW_PAGER))) {
		open_view(view, REQ_VIEW_DIFF, OPEN_SPLIT);
		split = 1;
	/* Always scroll the view even if it was split. That way
	 * you can use Enter to scroll through the log view and
	 * split open each commit diff. */
	scroll_view(view, REQ_SCROLL_LINE_DOWN);

	/* FIXME: A minor workaround. Scrolling the view will call report("")
	 * but if we are scrolling a non-current view this won't properly
	 * update the view title. */
	if (split)
		update_view_title(view);

/*
 * Main view backend
 */

struct commit {
	char id[41];		/* SHA1 ID. */
	char title[75];		/* The first line of the commit message. */
	char author[75];	/* The author of the commit. */
	struct tm time;		/* Date from the author ident. */
	struct ref **refs;	/* Repository references; tags & branch heads. */
};
main_draw(struct view *view, struct line *line, unsigned int lineno)
	struct commit *commit = line->data;
	size_t authorlen;
	int trimmed = 1;
	if (opt_utf8) {
		authorlen = utf8_length(commit->author, AUTHOR_COLS - 2, &col, &trimmed);
	} else {
		authorlen = strlen(commit->author);
		if (authorlen > AUTHOR_COLS - 2) {
			authorlen = AUTHOR_COLS - 2;
			trimmed = 1;
		}
	}

	if (trimmed) {
		waddnstr(view->win, commit->author, authorlen);
	col += AUTHOR_COLS;
main_read(struct view *view, struct line *prev, char *line)
		view->line[view->lines++].data = commit;
		if (!prev)
			break;

		commit = prev->data;

		if (!prev)
		commit = prev->data;

main_enter(struct view *view, struct line *line)
	enum open_flags flags = display[0] == view ? OPEN_SPLIT : OPEN_DEFAULT;

	open_view(view, REQ_VIEW_DIFF, flags);
	/**
	 * View manipulation
	 * ~~~~~~~~~~~~~~~~~
	 * q::
	 *	Close view, if multiple views are open it will jump back to the
	 *	previous view in the view stack. If it is the last open view it
	 *	will quit. Use 'Q' to quit all views at once.
	 * Enter::
	 *	This key is "context sensitive" depending on what view you are
	 *	currently in. When in log view on a commit line or in the main
	 *	view, split the view and show the commit diff. In the diff view
	 *	pressing Enter will simply scroll the view one line down.
	 * Tab::
	 *	Switch to next view.
	 * Up::
	 *	This key is "context sensitive" and will move the cursor one
	 *	line up. However, uf you opened a diff view from the main view
	 *	(split- or full-screen) it will change the cursor to point to
	 *	the previous commit in the main view and update the diff view
	 *	to display it.
	 * Down::
	 *	Similar to 'Up' but will move down.
	 **/
	{ 'q',		REQ_VIEW_CLOSE },
	{ KEY_UP,	REQ_PREVIOUS },
	{ KEY_DOWN,	REQ_NEXT },
	 * j::
	 *	Move cursor one line down.
	 * b::
	 * -::
	 * Space::
	{ 'k',		REQ_MOVE_UP },
	{ 'j',		REQ_MOVE_DOWN },
	{ ' ',		REQ_MOVE_PAGE_DOWN },
	{ 'b',		REQ_MOVE_PAGE_UP },
	{ '-',		REQ_MOVE_PAGE_UP },
	 * Q::
	 *	Quit.
	{ 'Q',		REQ_QUIT },
/*
 * Unicode / UTF-8 handling
 *
 * NOTE: Much of the following code for dealing with unicode is derived from
 * ELinks' UTF-8 code developed by Scrool <scroolik@gmail.com>. Origin file is
 * src/intl/charset.c from the utf8 branch commit elinks-0.11.0-g31f2c28.
 */

/* I've (over)annotated a lot of code snippets because I am not entirely
 * confident that the approach taken by this small UTF-8 interface is correct.
 * --jonas */

static inline int
unicode_width(unsigned long c)
{
	if (c >= 0x1100 &&
	   (c <= 0x115f				/* Hangul Jamo */
	    || c == 0x2329
	    || c == 0x232a
	    || (c >= 0x2e80  && c <= 0xa4cf && c != 0x303f)
						/* CJK ... Yi */
	    || (c >= 0xac00  && c <= 0xd7a3)	/* Hangul Syllables */
	    || (c >= 0xf900  && c <= 0xfaff)	/* CJK Compatibility Ideographs */
	    || (c >= 0xfe30  && c <= 0xfe6f)	/* CJK Compatibility Forms */
	    || (c >= 0xff00  && c <= 0xff60)	/* Fullwidth Forms */
	    || (c >= 0xffe0  && c <= 0xffe6)
	    || (c >= 0x20000 && c <= 0x2fffd)
	    || (c >= 0x30000 && c <= 0x3fffd)))
		return 2;

	return 1;
}

/* Number of bytes used for encoding a UTF-8 character indexed by first byte.
 * Illegal bytes are set one. */
static const unsigned char utf8_bytes[256] = {
	1,1,1,1,1,1,1,1, 1,1,1,1,1,1,1,1, 1,1,1,1,1,1,1,1, 1,1,1,1,1,1,1,1,
	1,1,1,1,1,1,1,1, 1,1,1,1,1,1,1,1, 1,1,1,1,1,1,1,1, 1,1,1,1,1,1,1,1,
	1,1,1,1,1,1,1,1, 1,1,1,1,1,1,1,1, 1,1,1,1,1,1,1,1, 1,1,1,1,1,1,1,1,
	1,1,1,1,1,1,1,1, 1,1,1,1,1,1,1,1, 1,1,1,1,1,1,1,1, 1,1,1,1,1,1,1,1,
	1,1,1,1,1,1,1,1, 1,1,1,1,1,1,1,1, 1,1,1,1,1,1,1,1, 1,1,1,1,1,1,1,1,
	1,1,1,1,1,1,1,1, 1,1,1,1,1,1,1,1, 1,1,1,1,1,1,1,1, 1,1,1,1,1,1,1,1,
	2,2,2,2,2,2,2,2, 2,2,2,2,2,2,2,2, 2,2,2,2,2,2,2,2, 2,2,2,2,2,2,2,2,
	3,3,3,3,3,3,3,3, 3,3,3,3,3,3,3,3, 4,4,4,4,4,4,4,4, 5,5,5,5,6,6,1,1,
};

/* Decode UTF-8 multi-byte representation into a unicode character. */
static inline unsigned long
utf8_to_unicode(const char *string, size_t length)
{
	unsigned long unicode;

	switch (length) {
	case 1:
		unicode  =   string[0];
		break;
	case 2:
		unicode  =  (string[0] & 0x1f) << 6;
		unicode +=  (string[1] & 0x3f);
		break;
	case 3:
		unicode  =  (string[0] & 0x0f) << 12;
		unicode += ((string[1] & 0x3f) << 6);
		unicode +=  (string[2] & 0x3f);
		break;
	case 4:
		unicode  =  (string[0] & 0x0f) << 18;
		unicode += ((string[1] & 0x3f) << 12);
		unicode += ((string[2] & 0x3f) << 6);
		unicode +=  (string[3] & 0x3f);
		break;
	case 5:
		unicode  =  (string[0] & 0x0f) << 24;
		unicode += ((string[1] & 0x3f) << 18);
		unicode += ((string[2] & 0x3f) << 12);
		unicode += ((string[3] & 0x3f) << 6);
		unicode +=  (string[4] & 0x3f);
		break;
	case 6:
		unicode  =  (string[0] & 0x01) << 30;
		unicode += ((string[1] & 0x3f) << 24);
		unicode += ((string[2] & 0x3f) << 18);
		unicode += ((string[3] & 0x3f) << 12);
		unicode += ((string[4] & 0x3f) << 6);
		unicode +=  (string[5] & 0x3f);
		break;
	default:
		die("Invalid unicode length");
	}

	/* Invalid characters could return the special 0xfffd value but NUL
	 * should be just as good. */
	return unicode > 0xffff ? 0 : unicode;
}

/* Calculates how much of string can be shown within the given maximum width
 * and sets trimmed parameter to non-zero value if all of string could not be
 * shown.
 *
 * Additionally, adds to coloffset how many many columns to move to align with
 * the expected position. Takes into account how multi-byte and double-width
 * characters will effect the cursor position.
 *
 * Returns the number of bytes to output from string to satisfy max_width. */
static size_t
utf8_length(const char *string, size_t max_width, int *coloffset, int *trimmed)
{
	const char *start = string;
	const char *end = strchr(string, '\0');
	size_t mbwidth = 0;
	size_t width = 0;

	*trimmed = 0;

	while (string < end) {
		int c = *(unsigned char *) string;
		unsigned char bytes = utf8_bytes[c];
		size_t ucwidth;
		unsigned long unicode;

		if (string + bytes > end)
			break;

		/* Change representation to figure out whether
		 * it is a single- or double-width character. */

		unicode = utf8_to_unicode(string, bytes);
		/* FIXME: Graceful handling of invalid unicode character. */
		if (!unicode)
			break;

		ucwidth = unicode_width(unicode);
		width  += ucwidth;
		if (width > max_width) {
			*trimmed = 1;
			break;
		}

		/* The column offset collects the differences between the
		 * number of bytes encoding a character and the number of
		 * columns will be used for rendering said character.
		 *
		 * So if some character A is encoded in 2 bytes, but will be
		 * represented on the screen using only 1 byte this will and up
		 * adding 1 to the multi-byte column offset.
		 *
		 * Assumes that no double-width character can be encoding in
		 * less than two bytes. */
		if (bytes > ucwidth)
			mbwidth += bytes - ucwidth;

		string  += bytes;
	}

	*coloffset += mbwidth;

	return string - start;
}


static bool cursed = FALSE;
	update_display_cursor();
/* Id <-> ref store */
static struct ref ***id_refs;
static size_t id_refs_size;

	struct ref ***tmp_id_refs;
	struct ref **ref_list = NULL;
	size_t ref_list_size = 0;
	for (i = 0; i < id_refs_size; i++)
		if (!strcmp(id, id_refs[i][0]->id))
			return id_refs[i];

	tmp_id_refs = realloc(id_refs, (id_refs_size + 1) * sizeof(*id_refs));
	if (!tmp_id_refs)
		return NULL;

	id_refs = tmp_id_refs;

		tmp = realloc(ref_list, (ref_list_size + 1) * sizeof(*ref_list));
			if (ref_list)
				free(ref_list);
		ref_list = tmp;
		if (ref_list_size > 0)
			ref_list[ref_list_size - 1]->next = 1;
		ref_list[ref_list_size] = &refs[i];
		ref_list[ref_list_size]->next = 0;
		ref_list_size++;
	if (ref_list)
		id_refs[id_refs_size++] = ref_list;

	return ref_list;
}

static int
read_ref(char *id, int idlen, char *name, int namelen)
{
	struct ref *ref;
	bool tag = FALSE;
	bool tag_commit = FALSE;

	/* Commits referenced by tags has "^{}" appended. */
	if (name[namelen - 1] == '}') {
		while (namelen > 0 && name[namelen] != '^')
			namelen--;
		if (namelen > 0)
			tag_commit = TRUE;
		name[namelen] = 0;
	}

	if (!strncmp(name, "refs/tags/", STRING_SIZE("refs/tags/"))) {
		if (!tag_commit)
			return OK;
		name += STRING_SIZE("refs/tags/");
		tag = TRUE;

	} else if (!strncmp(name, "refs/heads/", STRING_SIZE("refs/heads/"))) {
		name += STRING_SIZE("refs/heads/");

	} else if (!strcmp(name, "HEAD")) {
		return OK;
	}

	refs = realloc(refs, sizeof(*refs) * (refs_size + 1));
	if (!refs)
		return ERR;

	ref = &refs[refs_size++];
	ref->name = strdup(name);
	if (!ref->name)
		return ERR;

	ref->tag = tag;
	string_copy(ref->id, id);

	return OK;
	return read_properties(cmd, '\t', read_ref);
}
static int
read_config_option(char *name, int namelen, char *value, int valuelen)
{
	if (!strcmp(name, "i18n.commitencoding")) {
		string_copy(opt_encoding, value);
	}
	return OK;
}
static int
load_config(void)
{
	return read_properties("git repo-config --list", '=',
			       read_config_option);
}
static int
read_properties(const char *cmd, int separator,
		int (*read_property)(char *, int, char *, int))
{
	FILE *pipe = popen(cmd, "r");
	char buffer[BUFSIZ];
	char *name;
	int state = OK;
	if (!pipe)
		return ERR;
	while (state == OK && (name = fgets(buffer, sizeof(buffer), pipe))) {
		char *value = strchr(name, separator);
		int namelen;
		int valuelen;

		if (value) {
			namelen = value - name;
			*value++ = 0;
			valuelen = strlen(value);
			if (valuelen > 0) {
				valuelen--;
				value[valuelen] = 0;
			}
		} else {
			namelen = strlen(name);
			value = "";
			valuelen = 0;
		if (namelen)
			state = read_property(name, namelen, value, valuelen);
	if (state != ERR && ferror(pipe))
		state = ERR;
	return state;

#if __GNUC__ >= 3
#define __NORETURN __attribute__((__noreturn__))
#else
#define __NORETURN
#endif

static void __NORETURN
static void __NORETURN
die(const char *err, ...)
	/* Load the repo config file first so options can be overwritten from
	 * the command line.  */
	if (load_config() == ERR)
		die("Failed to load repo config.");

	/* Require a git repository unless when running in pager mode. */
	if (refs_size == 0 && opt_request != REQ_VIEW_PAGER)
		die("Not a git repository");

 * variables described in the  <<history-commands, "History commands">>
 * section.
 *	$ tig log Makefile README
 *	$ tig -- --after="May 5th" --before="2006-05-16 15:44"
 * NOTE: If you want to avoid having to quote dates containing spaces you
 * can use "." instead, e.g. `--after=May.5th`.
 * - In it's current state tig is pretty much UTF-8 only.
 *
 * - The cursor can wrap-around on the last line and cause the
 *   window to scroll.
 *
 * Copyright (c) 2006 Jonas Fonseca <fonseca@diku.dk>
 * - link:http://www.kernel.org/pub/software/scm/git/docs/[git(7)],
 * - link:http://www.kernel.org/pub/software/scm/cogito/docs/[cogito(7)]
 *
 * Other git repository browsers:
 *
 *  - gitk(1)
 *  - qgit(1)
 *  - gitview(1)