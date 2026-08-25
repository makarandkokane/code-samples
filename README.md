# code-samples

Self-contained demo projects: one folder per demo, grouped by technology. Every demo folder carries its own README with screenshots and build notes, so the screenshots are the demo if you don't feel like building anything.

By [Makarand Kokane](https://makarandkokane.github.io/).

## Demos

| Demo | Technology | What it shows |
|------|------------|---------------|
| [observer-mvc](qt/observer-mvc/) | Qt Widgets | One model, three synchronized views: the observer pattern as Qt ships it |
| [event-loop-thread](qt/event-loop-thread/) | Qt Widgets | Ten threads sell tickets, one event loop owns 100,000 seats, and there is not a lock in sight |
| [linked-list](data-structures/linked-list/) | Plain C | The classic linked list operations behind a menu: reverse, merge sort, palindrome check, the tortoise and hare, and a circle broken by the Josephus game |

## Requirements, design and tests

Every demo carries the three documents it was built from. They are cross-linked both ways: a requirement points at the design elements and the tests that carry it, each test points back at what it verifies, and the design points at the source files.

Start at a discipline: [Requirements](1_Requirements/index.html) &middot; [Design](2_Designs/index.html) &middot; [Tests](3_Tests/index.html). Or take one demo at a time:

| Demo | Documents |
|------|-----------|
| observer-mvc | [Requirements](1_Requirements/observer-mvc/requirements.html) &middot; [Design](2_Designs/observer-mvc/design.html) &middot; [Tests](3_Tests/observer-mvc/tests.html) |
| event-loop-thread | [Requirements](1_Requirements/event-loop-thread/requirements.html) &middot; [Design](2_Designs/event-loop-thread/design.html) &middot; [Tests](3_Tests/event-loop-thread/tests.html) |
| linked-list | [Requirements](1_Requirements/linked-list/requirements.html) &middot; [Design](2_Designs/linked-list/design.html) &middot; [Tests](3_Tests/linked-list/tests.html) |

<!-- Planned technology groups: qt/ qml/ modern-cpp/ vtk/ opengl/ -->
