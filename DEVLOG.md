# Dev Log:

## Michael Avrahami

### 2025-01-03 - Created proposal
Wrote proposal.md all steps. 1.5 hrs

### 2025-01-06 - Initialization
Created readme file for game instructions. 0.5 hrs
Wrote makefile. 0.33 hrs
Brainstormed and planned codewriting. 0.2 hrs

### 2025-01-07 - Planning and Pipe-Creation
Wrote out several function headers for the game progression. Thought about further steps. 0.5 hrs
Wrote initializeGame function which created the subservers for each player and sent them off to play. 1 hr

### 2025-01-08 - Subserver Interaction
Worked on subservers receiving information about their opponents and game setup. 1 hr
Began work on subservers piping to each other info. o.3 hrs

### 2025-01-09 - Absence
Absent from class, unable to work at home.

### 2025-01-10 - Restructure
Took down subserver pipe communication with each other. 0.2 hrs
Began working with semaphores and shared memory instead to communicate info between subservers. 1 hr

### 2025-01-11/12 - Accessing
Subservers worked to access their respective semaphores and shared memory segments to allow the game to begin. 1.5 hrs

### 2025-01-13 - User Interface
Wrote client to server communication via pipes - 0.5 hrs
Wrote client-side operations - reading from stdin and processing data 0.5 hrs

### 2025-01-14 - Shared Memory Restructuring
Wrote winning and losing code--telling the players whether they won or lost, what to do if that happens. 0.75 hrs
Used shared memory to indicate result to players - 0.2 hrs

### 2025-01-15 - Game ending
Wrote client-side and server-side instructions for how to proceed after a player answers incorrectly - 0.75 hrs
Completely restructured how server writes data to clients. 1.5 hrs

### 2025-01-16 - Next rounds + Debugging
After the first round finishes, variables reset and the next round begins with only the remaining players - 1.0 hrs
Debugged everything from bye players to player-side mechanics to winning screen - 2.0 hrs

### 2025-01-17 - Testing on Different Accounts
Much more debugging of the game this time involving having multiple different accounts able to connect to the program on the same computer, using chmod - 1.0 hrs

### 2025-01-19 - More different accounts
Switched infrastructure from server-side timing and "kill" to client-side timing and pipe-writing to ensure multi-account access - 1.5 hrs

### 2025-01-20 - Cleaned up
Cleaned up (closed pipes, freed malloced memory, detached variables and closed shared memory and semaphores, etc.) - 1.0 hrs
Created video presentation, debugged - 0.5 hrs
