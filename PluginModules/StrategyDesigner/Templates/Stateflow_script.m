rt = sfroot;
m = rt.find('-isa','Simulink.BlockDiagram','-and','Name','ModeleRobot');
ch = m.find('-isa','Stateflow.Chart');
%affichage du modèle
%ch.view;

objArray = ch.find('Name', 'DEMO');

