%%
% version history
% 
% 070716 - bug fixed: prevent printing 'CELLCENTERED or NODECENTERED'
% keyword if there are no such thing.

%%%
clear;

%% Make a TECPLOT output file 
% - files required (1grid.ugs, 1hgeo.ugs, 0ugs.bin)
gridfn = '1grid.ugs';  %grid file
hgeofn = '1hgeo.ugs';  %hydrogeologic group index (material zone)
binfn = '0ugs.bin';    %simulation results file

% - tecplot output file
tecoutfn = '0tec_out.plt';



% read a grid (0grid.tmp)
[nz, tn, te, X, Y, Z, EI] = read_grid (gridfn);



% read hydrogeology file (1hgeo.tmp)
fid = fopen(hgeofn,'r');
hgeo_type = fscanf (fid, '%2u');
fclose(fid);



% count elements of each type
cee = zeros(10,1);
for i = 1:10  % suppose number of material types are max 10
    cee(i) = sum(hgeo_type == i);
end

tmt = sum(cee~=0); % total number of material types 



%output filename
% fileID = fopen('0tec_grid.ugs','w');
fileID = fopen(tecoutfn,'w');


% variables and time you want to print.
% change below lists as you like.
varlist = {'X','Y','Z','H','HT','UX','UY','UZ','EV','Qwx_e','Qwy_e','Qwz_e','CFP','Sxx','Szz','P1_e','P3_e','Pb_e','DFX','DFY','DFZ'}; 
           % X,Y,Z must come first
varloc =  [1,1,1,1,1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0 ,0, 0, 1, 1, 1]; % 1: nodal, 0: elemental
timelist = [0 1200 2400 3600 4800 6000 7200];
% number of time step
[n,ntime] = size(timelist);
% number of variables
[m,nvar] = size(varloc);

% make a cell of variable arrays
DATA = cell(nvar+3,ntime);

DATA{1,1} = X;
DATA{2,1} = Y;
DATA{3,1} = Z;

% read data from 0ugs.bin and assign to the cell
for t = 1:ntime
    for i = 4:nvar
       DATA{i,t} = read_bin(binfn, varlist{i}, timelist(t));
    end
end



%% - print TECPLOT header
fprintf(fileID,'TITLE = "UGSM TECPLOT OUTPUT"\n');
fprintf(fileID,'\n');


% make a variable list
str = '';
for i = 1:nvar
    if i == 1
        str = varlist{i};
    else
        str = [str ', ' varlist{i}]; 

    end
end

fprintf(fileID,['VARIABLES = ' str '\n']);

% variable location
loc_n = find(varloc == 1);
loc_e = find(varloc == 0);

str_n = num2str(loc_n);
str_e = num2str(loc_e);

% format
formatSpecN = '%12.8e %12.8e %12.8e %12.8e %12.8e\n'; % for data
formatSpecEI = '%10u %10u %10u %10u %10u %10u %10u %10u\n'; % for elemental index

zone_count = 0;
% loop for time step
for t = 1:ntime
    disp(num2str(timelist(t)))
    
    % loop for every hydrogeologic group
    for mp = 1:tmt
        
        zone_count = zone_count + 1;
        te_zone = cee(mp);

        fprintf(fileID,'\n');
        fprintf(fileID,['ZONE T= "Zone ' num2str(mp) ' at ' num2str(timelist(t),'%8e') '", \n']);
        fprintf(fileID,'\tN = %10u, E = %10u,\n',tn,te_zone);
        fprintf(fileID,'\tZONETYPE= FEBRICK, \n');
        fprintf(fileID,'\tDATAPACKING= BLOCK, \n');
    %     fprintf(fileID,'\tSTRANDID = 1, \n');
        fprintf(fileID,['\tSOLUTIONTIME= ' num2str(timelist(t)) ', \n']); 
        if strcmp(str_n, '') == 0
            fprintf(fileID,['\tVARLOCATION= ([' str_n ']=NODAL)\n']);
        end
        if strcmp(str_e,'') == 0
            fprintf(fileID,['\tVARLOCATION= ([' str_e ']=CELLCENTERED)\n']);
        end

        %X,Y,Z
        if mp == 1
            if t == 1          
                for inv = 1:3 % print coordinates
                    fprintf(fileID,formatSpecN,DATA{inv,t});
                    fprintf(fileID,'\n');
                end
                
            else
                fprintf(fileID,'\tVARSHARELIST= ([1-3]= 1)\n'); %share variables 1 to 3 from zone 1
                fprintf(fileID,'\tCONNECTIVITYSHAREZONE= 1 \n'); %share FEM connectivity from zone 1  
            end
            
            for inv = 4:nvar % print data
                if varloc(inv) == 1
                    fprintf(fileID,formatSpecN,DATA{inv,t});
                else
                    ELE_VAR = DATA{inv,t}(hgeo_type == 1);
                    fprintf(fileID,formatSpecN,ELE_VAR);
                end    
                fprintf(fileID,'\n');
            end          
            
            if t == 1
                % Element Index
                for i = 1:te
                    if hgeo_type(i) == mp
                        fprintf(fileID,formatSpecEI,EI(i,1),EI(i,2),EI(i,3),EI(i,4),...
                                EI(i,5),EI(i,6),EI(i,7),EI(i,8));
                    end
                end
            end
        else
        % mp >= 2   
        
            ref_zone = zone_count - mp + 1;
            fprintf(fileID,['\tVARSHARELIST= ([' num2str(str_n) ']= ' num2str(ref_zone) ')\n']); 
                                            %share nodal variables from the 1st zone of each timestep.
                                            
            if t ~= 1
               fprintf(fileID,['\tCONNECTIVITYSHAREZONE= ' num2str(mp) '\n']); %share FEM connectivity from zone mp 
            end

            for inv = 4:nvar % print data
                if varloc(inv) == 0
                    ELE_VAR = DATA{inv,t}(hgeo_type == mp);
                    fprintf(fileID,formatSpecN,ELE_VAR);
                end    
            end          
                        
            % Element Index
            if t == 1
                formatSpecEI = '%10u %10u %10u %10u %10u %10u %10u %10u\n';
                for i = 1:te
                    if hgeo_type(i) == mp
                        fprintf(fileID,formatSpecEI,EI(i,1),EI(i,2),EI(i,3),EI(i,4),...
                                EI(i,5),EI(i,6),EI(i,7),EI(i,8));
                    end
                end    
            end

        end
    end
    fprintf(fileID,'\n');
end

disp('0tec_out.plt has written.');


fclose(fileID);

