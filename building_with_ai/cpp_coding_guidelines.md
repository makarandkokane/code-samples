# C++ coding guidelines

Every demo in this repo is built with AI, and this file is half of what makes that work: the C++ rules the AI writes under, and the checklist I review its output against before anything is published. The mechanical half is enforced by the [.clang-format](../.clang-format) at the repo root; the judgment half lives here. C demos follow the self-contained C counterpart, [c_coding_guidelines.md](c_coding_guidelines.md).

One idea drives all of it: demo code is read far more than it is run, so every rule trades a little writing effort for a lot of reading ease.

## Layout

**Braces.** Opening braces always start on their own line, in all code: functions, classes, control flow, even lambdas.

**If statements.** An if whose body is one single-line statement takes no braces, and a brace-less if without an else is followed by exactly one blank line (skipped when the chunk's closing brace follows anyway). A body statement that wraps across lines keeps its braces, and loops always keep theirs.

**Blank lines are the chunking tool.** Inside a function, group statements into chunks of one job each and separate the chunks with exactly one blank line: never two, never straight after the opening brace, never just before the closing one. A comment that introduces a chunk belongs to it: blank line above the comment, none between the comment and the first statement it describes. The final chunk needs no trailing blank, the closing brace already ends it. clang-format enforces the mechanical half; where the chunk boundaries fall is a human call.

**Vertical alignment.** Runs of declarations or assignments with nothing between them line up as columns. clang-format does this mechanically; a blank line or a comment starts a fresh group, so each group aligns within itself. Literal tables the tool cannot align are aligned by hand, numbers right-aligned, inside clang-format off/on guards.

**Initializer lists.** Never packed onto one line. Every constructor initializer gets its own line, the colon leading the first at 4 spaces, commas trailing, the rest aligned under it at 6 spaces. This applies even when there is only one initializer.

## Comments

**A comment earns its place** only by stating what the code cannot: a role in the design, a contract, or a why. Nothing whose content is obvious from the code; "} // namespace" and the like read amateur, so namespace end comments are switched off in .clang-format.

**Function comments.** Every function definition in a .cpp opens with at least a one-line comment saying what the function is for; a non-trivial one also covers inputs, outputs and the return value.

**Chunk headings.** Each chunk inside a longer function opens with one short line saying what that chunk produces (the name column, the bar, the value), and where there is a why worth having it carries it too ("elided if the column is too narrow", "capped so the bars keep their space"). One line, never a paragraph. This is what makes a demo readable top to bottom, and it is the one place a mildly obvious comment is welcome.

## Names and numbers

**Constants.** A number that tunes something (a size, a padding, a gap, a delay, a range bound) is never written inline in a function body. It gets a named constexpr in an anonymous namespace at the top of the .cpp, k-prefixed and CamelCase: kRowHeight, kValueGap, kMinBarWidth. Loop bounds, index guards, accumulator seeds and arithmetic identities stay as digits, and so does literal demo data. The aim is that a reader meets a name, not an unexplained figure, in the middle of a function. In prose these are called unnamed numbers; the usual jargon term for them appears nowhere in this repo.

**Qualified names.** At the point of use, a name announces where it comes from: enum values are always written with their enum-name qualifier (Column::ColumnCount, ItemModel::Column::NameColumn), never bare. The reader should not have to remember which scope owns an identifier.

**File names.** A source file is named exactly after the class it holds: MainWindow.h/.cpp, BarChartWidget.h/.cpp (main.cpp, the entry point, stays lowercase). Include directives must match that casing exactly, so clones build on case-sensitive filesystems.

## Structure

**Headers hold declarations only.** No function bodies in a .h, not even a one-line accessor. Every definition lives in the matching .cpp, so a header reads as the class's contract and nothing else. Type definitions (enums, small structs the class exposes) do of course stay in the .h.

**Member defaults.** Every data member carries its initializer where it is declared in the .h, so none is ever left indeterminate. A member whose real value arrives from the constructor (a parameter, or a new) still declares = nullptr in the header and takes its actual value in the constructor's list; the header default is the safety net, not the wiring. Never move a new into the header just to satisfy this: that would force a full include where a forward declaration was enough. Class-type members with their own default constructor need nothing.

**Member groups.** Header data members stand in small, tightly related groups, each opened by a one-line comment saying what that group is for, one blank line between groups.

**Small functions.** A function doing several jobs gets split into helpers, one job each (paintEvent = compute layout + draw rows; a window constructor = createViews + createToolbar). Placement follows what the helper touches: a pure computation (inputs to result, no object state) becomes a file-local free function in an anonymous namespace, so its signature proves what it touches; a helper that needs the object's state (palette, model, child widgets) becomes a private member function.

**Readability over compression.** Split dense expressions into multiple simple statements with well-named local variables (compute name and value first, then append({name, value})), and prefer plain if/return over packed ternaries and nested calls.

## Enforcement

The [.clang-format](../.clang-format) at the repo root encodes everything a formatter can encode. New sources get a clang-format pass before publishing; the rest of this file is applied by review.
