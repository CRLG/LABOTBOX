rt = sfroot;
m = rt.find('-isa','Simulink.BlockDiagram','-and','Name','ModeleRobot');
ch = m.find('-isa','Stateflow.Chart');
%affichage du mod�le
%ch.view;

objArray = ch.find('Name', 'DEMO');

