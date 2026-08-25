# Qt coding guidelines

These rules sit on top of [cpp_coding_guidelines.md](cpp_coding_guidelines.md) and cover what is specific to Qt. Same arrangement: the AI builds under them, I review against them.

## Class layout

**Q_OBJECT** is always followed by one blank line.

## Observing a subject

**Subscribe to destruction too.** A view that observes a subject connects to its destroyed() signal, not only its data-change signals. Qt emits destroyed() from ~QObject, so the handler may only clear the stored pointer, never dereference it.

**Gone is not empty.** Once the subject has died, the view renders an explicit gone state that is worded differently from its empty state: "Model deleted" is not "No items". QPointer is the wrong tool here: it silences the crash but collapses "the subject is gone" into "the subject is empty", and that distinction is exactly what a view should keep. Qt itself takes this route: QAbstractItemViewPrivate declares modelDestroyed() beside rowsInserted(), and keeps a plain model pointer while reserving QPointer for the delegate.

## Editing affordances

**An in-place editor must never crowd out the value it edits.** In a narrow column the stock spin box arrows do exactly that, so such a column gets a delegate that strips them and leaves plain typing (integers only, courtesy of the base editor). Watch the defaults when doing this: QSpinBox stops at 99 unless the range is widened, and would silently clamp a larger entry.

## Threads and event loops

**Shutdown order.** Shut a worker thread down before destroying the objects it replies to; stop it last and its queued replies land on objects that are already gone.

**Batch view updates onto a heartbeat.** When signals arrive at a high rate, the view repaints on a timer heartbeat instead of once per signal; per-signal repaints are what make a busy UI crawl.
