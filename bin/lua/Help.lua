print("  sourcing help.lua")

Simulation.HelpMessage = [===[
\monoface{}
ESC........... Quit program.
PAUSE......... Pause.

F1............ Show this help.
[SHIFT+] F2... Remove [add] prey [predators]s
[SHIFT+] F3... Decrease [increase] boundary radius
[SHIFT+] F4... Show next [previous] RTree level
F5............ Show cluster
F7............ Hide text.        
F8............ Fullscreen mode.

Cursor:............................. Camera rotation.
NUM-PAD '2','4','8','6':............ Camera move in plane.
NUM-PAD '9','3':.................... Camera move up, down.
[ALT+] PGUP, PGDOWN, Mouse-wheel:... Camera move in [zoom].
'1'................................. Fixed camera.
[SHIFT+][ALT+] '2'.................. TV camera (following selected prey [predator]).
[SHIFT+][ALT+] '3'.................. Observer camera (fly with selected prey [predator] on its path).
[SHIFT+][ALT+] '4'.................. First person camera prey [predator].
[SHIFT+][ALT+] '5'.................. Lateral local camera (fly by prey [predator]).
[SHIFT+][ALT+] '6'.................. Top camera (fly by prey [predator]).

Left-Click........... Select new target at mouse position.
SHIFT + Left-Click... Select new target at mouse position and toggle trail.
Left-DBL-Click....... Remove all marks.
    
'a'......... Enable/Disable alpha masking.
'b'......... Show/hide boundary.
ALT+'b'..... Toggle bakcground.
'c'......... show/hide circularity vector.
'f'......... Show/hide forces for the marked birds.
'h'......... Show/hide histogram.
SHIFT+'h'... Reset histogram. 
'v'......... Show/hide statistical numbers.
'l'......... Show/hide local coordinate system.
'm'......... Maximize boundary radius.
SHIFT+'m'... Minimize boundary radius.
CTRL+'m'.... Select next 3D model.
'n'......... Show/hide neighbors for the marked birds.
'p'......... Show/hide predator targeting.
SHIFT+'p'... Start attack.
ALT+'p'..... Stop attack.
'r'......... Show/hide search box for the marked birds.
's'......... Slow motion.
't'......... Show/hide trail.
'w'......... Toggle wireframe mode.

SHIFT+ENTER... Reload steering parameter (use sim.EnableReload(true)).
CTRL+'S'...... Save snapshot.
F11........... Save current statistic.
SHIFT+F11..... Pause/resume current statistic.
CTRL+F11...... Reset current statistic.
CTRL+DELETE... Delete invisible prey.
CTRL+ALT+R.... Reset simulation time.

(c) Hanno Hildenbrandt 2012
    h.hildenbrandt@rug.nl
]===]


