# C coding guidelines

Every demo in this repo is built with AI, and this file is half of what makes that work for the C demos: the rules the AI writes under, and the checklist I review its output against before anything is published. It is the C counterpart of [cpp_coding_guidelines.md](cpp_coding_guidelines.md): the shared rules restated in C terms, deliberately duplicated so a C demo needs only this file. The mechanical half is enforced by the [.clang-format](../.clang-format) at the repo root, which formats C sources with the same settings; the judgment half lives here.

One idea drives all of it: demo code is read far more than it is run, so every rule trades a little writing effort for a lot of reading ease.

## Layout

**Braces.** Opening braces always start on their own line, in all code: functions, control flow, struct and enum definitions.

**If statements.** An if whose body is one single-line statement takes no braces, and a brace-less if without an else is followed by exactly one blank line (skipped when the chunk's closing brace follows anyway). A body statement that wraps across lines keeps its braces, and loops always keep theirs.

**Blank lines are the chunking tool.** Inside a function, group statements into chunks of one job each and separate the chunks with exactly one blank line: never two, never straight after the opening brace, never just before the closing one. A comment that introduces a chunk belongs to it: blank line above the comment, none between the comment and the first statement it describes. The final chunk needs no trailing blank, the closing brace already ends it. clang-format enforces the mechanical half; where the chunk boundaries fall is a human call.

**Vertical alignment.** Runs of declarations or assignments with nothing between them line up as columns. clang-format does this mechanically; a blank line or a comment starts a fresh group, so each group aligns within itself. Literal tables the tool cannot align are aligned by hand, numbers right-aligned, inside clang-format off/on guards.

**Struct initializers.** A designated initializer that sets more than a couple of fields is never packed onto one line: one field per line, in declaration order.

## Comments

**A comment earns its place** only by stating what the code cannot: a role in the design, a contract, or a why. Nothing whose content is obvious from the code; a comment that restates its own line reads amateur.

**Function comments.** Every function definition in a .c opens with at least a one-line comment saying what the function is for; a non-trivial one also covers inputs, outputs and the return value.

**Chunk headings.** Each chunk inside a longer function opens with one short line saying what that chunk produces (the name column, the bar, the value), and where there is a why worth having it carries it too ("elided if the column is too narrow", "capped so the bars keep their space"). One line, never a paragraph. This is what makes a demo readable top to bottom, and it is the one place a mildly obvious comment is welcome.

## Names and numbers

**Constants.** A number that tunes something (a size, a padding, a gap, a delay, a range bound) is never written inline in a function body. It gets a name at the top of the .c: a file-scope static const, k-prefixed and CamelCase (kRowHeight, kValueGap, kMinBarWidth), or an enum constant where the language demands a constant expression (an array size, a case label). Loop bounds, index guards, accumulator seeds and arithmetic identities stay as digits, and so does literal demo data. The aim is that a reader meets a name, not an unexplained figure, in the middle of a function. In prose these are called unnamed numbers; the usual jargon term for them appears nowhere in this repo.

**Prefixed enumerators.** C pours every enumerator into the enclosing scope, so each one starts with its enum's name (enum Column { ColumnName, ColumnValue, ColumnCount }). The reader should not have to remember which enum owns an identifier.

**File names.** A source file is named exactly after the module it holds: RingBuffer.h/.c for a ring buffer module (main.c, the entry point, stays lowercase). Include directives must match that casing exactly, so clones build on case-sensitive filesystems.

## Structure

**Headers hold declarations only.** No function bodies in a .h. A header carries exactly the module's contract: the types it exposes and the functions callers may use. Everything else lives in the .c and is declared static, so it never leaks into the contract.

**No indeterminate fields.** Every struct instance is fully initialized at the point it is created, by a designated initializer or by an init function that sets every field. A field whose real value arrives later still starts at 0 or NULL; the default is the safety net, not the wiring.

**Struct field groups.** The fields of a struct stand in small, tightly related groups, each opened by a one-line comment saying what that group is for, one blank line between groups.

**Small functions.** A function doing several jobs gets split into static helpers, one job each. A helper takes what it needs as parameters rather than reaching for file-scope state, so its signature proves what it touches.

**Readability over compression.** Split dense expressions into multiple simple statements with well-named local variables (compute name and value first, then pass {name, value} on), and prefer plain if/return over packed ternaries and nested calls.

## Enforcement

The [.clang-format](../.clang-format) at the repo root encodes everything a formatter can encode, for .c and .h alike. New sources get a clang-format pass before publishing; the rest of this file is applied by review.
