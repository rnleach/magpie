# magpie
Memory management library in C11

Magpies are very smart birds, with a very good memory. They can remember a place they found food once (like my deck) and return there intermittently for years to the exact spot they found it and check to see if there is food there again. In the case of my deck, they've been returning for over 5 years as I write this, even though it's been that long since I left unattended food out there. Researchers have also shown that they remember specific people and how they treated them, allowing people and magpies to form (good or bad) relationships.

This really has nothing to do with computers or software, but I thought the loose association with was reason enough to name my memory management library magpie.

## Goals
 - Provide a common allocator interface for use in my projects.
 - Provide a cross platform (Windows, Linux, Mac) layer for getting memory from the OS to bypass malloc/free.
 - Create a variety of allocators that spans from simple and fast to more complicated and general so I can choose the best allocator for any specific task.
 - Build static (pre-determined size) and dynamic (unbounded) allocators.
 - Interoperate with my [Elk](https://github.com/rnleach/elk) library and my [Coyote](https://github.com/rnleach/coyote) library. Those libraries will depend on this one.
 - Single header library to keep it simple to use and integrate with my programs.
 - Reentrant functions and no global, mutable state.

## Non-goals
 - Create every allocator I *might eventually* need. I only build what I need, so if it's in there, I've used it.
 - Interoperate with malloc/free.


## Releases

### Version 1.0.0 - In progress.
 - XXXX-XX-XX (Initial Release)
