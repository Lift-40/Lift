# Lift
Main code repository for the lift project.


INSTRUCTIONS:

For opening 1st elevator simulator: 
	$rdmd sim_server.d
Then in elev_algo do 
	$make 
and for running the elevator
	$./ttk4145demoelevator


For opening a 2st elevator simulator, first change port number in elev_algo/simulator.con
	com_port              15657 -> 15658
	$rdmd sim_server.d ../../elev_algo/simulator.con
and for running the elevator
	$./ttk4145demoelevator


Do the same for opening a 3rd elevator simulator


DEFAULT KEYBOARD CONTROLS
Up: 0->q  1->w  2->e
Down: 1->s  2->d  3->f
Cab: 0->z  1->x  2->c  3->v
