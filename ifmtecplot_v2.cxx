#include <ifm/module.h>
#include <ifm/graphic.h>
#include <ifm/document.h>
#include <ifm/archive.h>
#include <stdio.h>
#include <stdlib.h>

/* --- IFMREG_BEGIN --- */
/*  -- Do not edit! --  */

static void PreSimulation (IfmDocument);

static const char szDesc[] =
  "ifm_tecplot_v2";

#ifdef __cplusplus
extern "C"
#endif /* __cplusplus */
#ifdef WIN32
__declspec(dllexport)
#endif /* WIN32 */

IfmResult RegisterModule(IfmModule pMod)
{
  if (IfmModuleVersion (pMod) < IFM_CURRENT_MODULE_VERSION)
    return False;
  IfmRegisterModule (pMod, "SIMULATION", "IFMTECPLOT_V2", "Ifmtecplot_v2", 0x1000);
  IfmSetDescriptionString (pMod, szDesc);
  IfmSetCopyrightPath (pMod, "ifmtecplot_v2.txt");
  IfmSetHtmlPage (pMod, "ifmtecplot_v2.htm");
  IfmSetPrimarySource (pMod, "ifmtecplot_v2.cxx");
  IfmRegisterProc (pMod, "PreSimulation", 1, (IfmProc)PreSimulation);
  return True;
}
/* --- IFMREG_END --- */

static void PreSimulation (IfmDocument pDoc)
{
  /*
   * TODO: Add your own code here ...
   */
   /* Declaration */
 	int i,j;

 	int NumNodes;
 	int NumElements;

 	FILE* fp = NULL;
 	char tecfile1[200] = "";
  char tecfile2[200] = "";


 	/* Assignment */
 		//get number of nodes and elements
 	NumNodes = IfmGetNumberOfNodes(pDoc);
 	NumElements = IfmGetNumberOfElements(pDoc);
 		//get nodewise properties
 	double xcoord[NumNodes],ycoord[NumNodes],zcoord[NumNodes];

 	for(i=0; i<NumNodes; i++){
 		xcoord[i] = IfmGetX(pDoc,i);
 		ycoord[i] = IfmGetY(pDoc,i);
 		zcoord[i] = IfmGetZ(pDoc,i);
 	}


 		//get elementwise properties
 	int elind[NumElements][8];
  int mp[NumElements];

 	for(i=0; i<NumElements; i++){
 		for (j=0;j<6;j++){
 			elind[i][j] = IfmGetNode(pDoc,i,j)+1;
 		}
 		elind[i][7]=elind[i][5]; //adjustment prism--> block
 		elind[i][6]=elind[i][5];
 		elind[i][5]=elind[i][4];
 		elind[i][4]=elind[i][3];
 		elind[i][3]=elind[i][2];
 	}

  for(i=0; i<NumElements; i++){
    mp[i] = IfmGetElementalRefDistrValue(pDoc,0,i);//take the first elemental distribution for MP
    // get the maximum value of mp
    max_mp = mp[0];
    if(max < mp[i]){
      max_mp = mp[i];
    }
  }


 		//get directory name and assign file name
  strcat(tecfile1, IfmGetFileDirectory(pDoc,IfmGetProblemPath(pDoc)));
 	strcat(tecfile1, "tecgrid.dat");


 	/* Show in the info box */
  IfmInfo(pDoc, "IFM_Tecplot_v2.0");
 	IfmInfo(pDoc, "Number of Nodes = %li\n", NumNodes);
 	IfmInfo(pDoc, "Number of Elements = %li\n", NumElements);


 	/* Open a file */
 	fp = fopen (tecfile, "wt");

 	/* Tecplot header */
 	fprintf(fp, "TITLE = \"FEFLOW GRID\"\n");
 	fprintf(fp, "VARIABLES = \"X\",\"Y\",\"Z\",\"MP\"\n");

 	/* Tecplot zone */

 	for (i=1; i<2; i++){
 		fprintf(fp, "ZONE T = \"ZONE  %2i\",\n",i);
 		fprintf(fp, "\tN = %10i, E = %10i,\n", NumNodes,NumElements);
 		fprintf(fp, "\tZONETYPE= FEBRICK,\n");
 		fprintf(fp, "\tDATAPACKING= BLOCK,\n");
 		fprintf(fp, "\tSOLUTIONTIME= 0,\n");
 		fprintf(fp, "\tVARLOCATION= ([1-3]=NODAL,[4]=CELLCENTERED)\n");
 	}

 	/* Tecplot data blocks */
 		//x-coordinates
 	for (i=0; i<NumNodes; i++){
 			fprintf(fp, "%.10e\n", xcoord[i]);
 	}
 		//y-coordinates
 	for (i=0; i<NumNodes; i++){
 			fprintf(fp, "%.10e\n", ycoord[i]);
 	}
 		//z-coordinates
 	for (i=0; i<NumNodes; i++){
 			fprintf(fp, "%.10e\n", zcoord[i]);
 	}

   //material property indexes
   for (i=0; i<NumElements; i++){
      fprintf(fp, "%3i\n", mp[i]);
   }

 		//element indexes
 	for (i=0; i<NumElements; i++){
 		for(j=0;j<8;j++){
 			fprintf(fp,"%10i ",elind[i][j]);
 		}
 		fprintf(fp, "\n");
 	}

 	fclose(fp);


  /*====================================================
  Zonal representation of the results
  - make tecplot ZONES according to MP
  =====================================================*/

// 20210115- Zonal out may not be very neccessary at this time. this will increase complexity only.
// just code this only for the 1st time step.


  IfmInfo(pDoc, "Saving a tecplot output with zones ...");

  // open a different file
  strcat(tecfile2, IfmGetFileDirectory(pDoc,IfmGetProblemPath(pDoc)));
  strcat(tecfile2, "teczone.dat");
  fp = fopen(tecfile2,"wt");

  /* Tecplot header */
  fprintf(fp, "TITLE = \"FEFLOW GRID\"\n");
  fprintf(fp, "VARIABLES = \"X\",\"Y\",\"Z\",\"NODE\",\"ELEMENT\"\n");

  /* Tecplot zone */

  // max_mp = number of zone -1
  for (i=1; i<max_mp; i++){
    fprintf(fp, "ZONE T = \"ZONE  %2i\",\n",i);
    fprintf(fp, "\tN = %10i, E = %10i,\n", NumNodes,NumElements);
    fprintf(fp, "\tZONETYPE= FEBRICK,\n");
    fprintf(fp, "\tDATAPACKING= BLOCK,\n");
    fprintf(fp, "\tSOLUTIONTIME= 0,\n");
    fprintf(fp, "\tVARLOCATION= ([1-4]=NODAL)\n");
    fprintf(fp, "\tVARLOCATION= ([5]=CELLCENTERED)\n");


  /* Tecplot data blocks */
      //x-coordinates
    for (j=0; j<NumNodes; j++){
        fprintf(fp, "%.10e\n", xcoord[j]);
    }
      //y-coordinates
    for (j=0; j<NumNodes; j++){
        fprintf(fp, "%.10e\n", ycoord[j]);
    }
      //z-coordinates
    for (j=0; j<NumNodes; j++){
        fprintf(fp, "%.10e\n", zcoord[j]);
    }


      //material property indexes
    for (j=0; j<NumElements; j++){
        fprintf(fp, "%3i\n", mp[j]);
    }

    //element indexes
    for (i=0; i<NumElements; i++){
      for(j=0;j<8;j++){
        fprintf(fp,"%10i ",elind[i][j]);
      }
      fprintf(fp, "\n");
    }

  } // end of mp loop

  fclose(fp);



  // end of PreSimulation
 	IfmInfo(pDoc, "Done:PreSimulation()");

}
