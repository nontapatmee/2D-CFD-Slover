#include <iostream>
#include <cmath>
#include <fstream>
#include <stdlib.h>
#include <unistd.h>
#include <string>
#include <complex>

using namespace std;

void initialize(double **u, double **u_new, double **v, double **v_new, double **F, double **G, double **P, double **P_new, double **Phi, double **Phi_new, int nx, int ny){
  
  for(int i=0; i<=nx-1; i++){
    for(int j=0; j<=ny-1; j++){
      P[i][j] =	0.;
      P_new[i][j] = 0.;
            
    }
  }
  for(int i=0; i<=nx-1; i++){
    for(int j=0; j<=ny-2; j++){
      G[i][j] = 0.;
      v[i][j] =	0.;
      v_new[i][j] = 0.;
      
    }
  }
  for(int i=0; i<=nx-2; i++){
    for(int j=0; j<=ny-1; j++){
      F[i][j] = 0.;
      u[i][j] = 0.;
      u_new[i][j] = 0.;  
    }
  }
  for(int i=0; i<=nx-2; i++){
    for(int j=0; j<=ny-2; j++){
      Phi[i][j] = 0.;
      Phi_new[i][j] = 0.;
      
      }
  }
  //set inlet boundary
  for(int j=1; j<=ny-2; j++){
    u[0][j] = 1.;
    u_new[0][j] = 1.;
    F[0][j] = 1;
  }
  //set boundary condition
  for(int i=0; i<=nx-2; i++){
    F[i][0]=(-1)*F[i][1];
    F[i][ny-1]=(-1)*F[i][ny-2];
    u[i][0]=(-1)*u[i][1];
    u[i][ny-1]=(-1)*u[i][ny-2];
    u_new[i][0]=(-1)*u_new[i][1];
    u_new[i][ny-1]=(-1)*u_new[i][ny-2];
  }
  //set polluted inlet boundary
  for(int j=1; j<=((ny-3)/2); j++){
    Phi[0][j] = 1.;
    Phi_new[0][j] = 1.;
  }
  

  
  
}

void visualize(double **var, int nx, int ny){

  for(int i=0; i<=nx-1; i++){
    for(int j=0; j<=ny-1; j++){
      cout << var[i][j] << "\t";
    }
    cout << "\n";
  }
  cout << "\n";
  
}

void update(double **var, double **var_new, int nx, int ny){
  for(int i = 0; i <= nx-1; i++){
    for(int j = 0; j <= ny-1; j++){    
      var[i][j] = var_new[i][j];
    }
  }
}

//step 1 : find F&G
void simulation_FG(double **u, double **u_new, double **v, double **v_new, double **F, double **G, int nx, int ny, double Re, double dx, double dy, double dt){
  double RHS_F;
  double RHS_F1;
  double RHS_F2;
  double RHS_F3;

  double RHS_G;
  double RHS_G1;
  double RHS_G2;
  double RHS_G3;
  double gamma;
 
  for(int i=1; i<=nx-3; i++){
    for(int j =1; j<=ny-2; j++){
      RHS_F1 = (((u[i+1][j]-2*u[i][j]+u[i-1][j])/pow(dx,2.))+((u[i][j+1]-2*u[i][j]+u[i][j-1])/pow(dy,2.)))/Re;
      RHS_F2 = (pow(((u[i][j]+u[i+1][j])/2),2.)-pow(((u[i-1][j]+u[i][j])/2),2.))/dx;
      RHS_F3 = (((v[i][j]+v[i+1][j])/2)*((u[i][j]+u[i][j+1])/2)-((v[i][j-1]+v[i+1][j-1])/2)*((u[i][j-1]+u[i][j])/2))/dy;
      RHS_F = RHS_F1  - RHS_F3 -RHS_F2;
      F[i][j] = u[i][j] + dt*RHS_F;
      
    }
  }
  
  //find G
  for(int i=1; i<=nx-2; i++){
    for(int j =1; j<=ny-3; j++){
      RHS_G1 = (((v[i+1][j]-2*v[i][j]+v[i-1][j])/pow(dx,2.))+((v[i][j+1]-2*v[i][j]+v[i][j-1])/pow(dy,2.)))/Re;
      RHS_G3 = (pow(((v[i][j]+v[i][j+1])/2),2.)-pow(((v[i][j-1]+v[i][j])/2),2.))/dy;
      RHS_G2 = (((u[i][j]+u[i][j+1])/2)*((v[i][j]+v[i+1][j])/2)-((u[i-1][j]+u[i-1][j+1])/2)*((v[i-1][j]+v[i][j])/2))/dx;
      RHS_G = RHS_G1 - RHS_G2 - RHS_G3;
      G[i][j] = v[i][j] + dt*RHS_G;
      
    }
  }

  //update inlet, outlet G,F boundary
  //G inlet Dirichlet, outlet neumann
  for(int j = 1; j<=ny-3; j++){
    G[0][j]=(-1)*G[1][j];
    G[nx-1][j]=G[nx-2][j];
  }
  //F outlet neumann, wall Dirichlet
  for(int j =1; j<=ny-2; j++){
    F[nx-2][j]=F[nx-3][j];
  }
  for(int i=0; i<=nx-2; i++){
    F[i][0]=(-1)*F[i][1];
    F[i][ny-1]=(-1)*F[i][ny-2];
  }
  
}


//step 2 find P[i,j] : Solving Poisson equation using gauss seidel iteration
void simulation_p(double **u, double **u_new, double **v, double **v_new, double **F, double **G, double **P, double **P_new, int nx, int ny, double Re, double dx, double dy, double dt){
  double aa, bb, cc;
  double ee, ew, en, es;
  double RHS;
  bool status = true;
  aa = 1./pow(dx,2.);
  bb = 1./pow(dy,2.);
	
  double w=1.;
  double rit=1.;
  double eps=0.01;
  int it, it_max;
  it_max=1000;
  
  //update bd condition F(0,j) <= u(0,j) ,v(i,0) => G(i,0)
  for(int j=0; j<=ny-1; j++){
     F[0][j]=u[0][j];
     F[nx-2][j]=u[nx-2][j];
   }
   for(int i=0; i<=nx-1; i++){
     G[i][0]=v[i][0];
     G[i][ny-2]=v[i][ny-2];
   }
   it=0;

   while(true){
     //check convergence ,norm of the residual<eps and max iteration
    if(status == false || (rit<eps) || (it>it_max) ){break;}

    rit =0.;
    
    for(int i=1; i<=nx-2; i++){
      for(int j =1; j<=ny-2; j++){

	if(i==1){ew=0;}
        else{ew=1;}
        if(i==nx-2){ee=0;}
        else{ee=1;}
        if(j==1){es=0;}
        else{es=1;}
        if(j==ny-2){en=0;}
        else{en=1;}

        cc = (pow(dx,2.)*pow(dy,2.))/((en+es)*pow(dx,2.)+(ee+ew)*pow(dy,2.));
        P_new[i][j]=(  ( (((F[i][j]-F[i-1][j])/dx) + ((G[i][j]-G[i][j-1])/dy))*(-1./dt) )  +  ( aa*ee*P[i+1][j] + aa*ew*P_new[i-1][j] + bb*en*P[i][j+1] + bb*es*P_new[i][j-1] )  )*cc;
	RHS=((F[i][j]-F[i-1][j])/dx + (G[i][j]-G[i][j-1])/dy)/dt;
        rit=rit+pow( (ee*(P[i+1][j]-P[i][j])-ew*(P[i][j]-P[i-1][j]))/pow(dx,2) + (en*(P[i][j+1]-P[i][j])-es*(P[i][j]-P[i][j-1]))/pow(dy,2) - RHS,2);
	status = false;
      }
    }
    
    rit=sqrt(rit/((nx-2)*(ny-2)));

    //check convergence
    for(int i=1; i<=nx-2; i++){
      for(int j =1; j<=ny-2; j++){
	if(abs(P[i][j] - P_new[i][j]) > 10e-50){
	  status = true;
	}
      }
    }

    //update P wall boundary
    for(int j=0; j<=ny-1; j++){
      P_new[0][j] = P_new[1][j];
      P_new[nx-1][j] = P_new[nx-2][j];
    }
    for(int i=0; i<=nx-1; i++){
      P_new[i][0] = P_new[i][1];
      P_new[i][ny-1]=P_new[i][ny-2];
    }

    //update P_new=>P
    for(int i=0; i<=nx-1; i++){
      for(int j =0; j<=ny-1; j++){
    	P[i][j]=P_new[i][j];
      }
    }

    it++;

   }
  cout<<rit<<"\n";
 
}


//step3 find uv
void simulation_uv(double **u, double **u_new, double **v, double **v_new, double **F, double **G, double **P, double **P_new, int nx, int ny, double Re, double dx, double dy, double dt){

  for(int i=1; i<=nx-3; i++){
      for(int j =1; j<=ny-2; j++){
	u_new[i][j]=F[i][j]-(dt*(P_new[i+1][j]-P_new[i][j]))/dx;
      }
  }	
  for(int i=1; i<=nx-2; i++){
      for(int j =1; j<=ny-3; j++){
	v_new[i][j]=G[i][j]-(dt*(P_new[i][j+1]-P_new[i][j]))/dy;
      }
  }  	

 //update u,v boundary
 for(int j =1; j<=ny-2; j++){
   u_new[0][j]=1;
 }
 for(int j=0; j<=ny-1; j++){
   u_new[nx-2][j]=u_new[nx-3][j];
 }
 for(int j=1; j<=ny-3; j++){
   v_new[0][j]=(-1)*v_new[1][j];
   v_new[nx-1][j]=v_new[nx-2][j];
 }
 for(int i=0; i<=nx-2; i++){
   u_new[i][0]=(-1)*u_new[i][1];
   u_new[i][ny-1]=(-1)*u_new[i][ny-2];
 }
 
}

//find passive scalar using explicit euler
void simulation_passiveScalar(double **u, double **u_new, double **v, double **v_new, double **F, double **G, double **P, double **P_new, double **Phi, double **Phi_new, int nx, int ny, double Re, double dx, double dy, double dt){
  for(int i = 1; i <= nx-3; i++){
    for(int j = 1; j <= ny-3; j++){    
      Phi_new[i][j] = ((((Phi[i+1][j]-2*Phi[i][j]+Phi[i-1][j])/(pow(dx,2)*Re))-((u[i][j]/(2*dx))*(Phi[i+1][j]-Phi[i-1][j])))+(((Phi[i][j+1]-2*Phi[i][j]+Phi[i][j-1])/(pow(dy,2)*Re))-((v[i][j]/(2*dy))*(Phi[i][j+1]-Phi[i][j-1]))))*dt+Phi[i][j];
     }
  }
  for(int j = 1; j <= (ny-3); j++){    
      Phi_new[nx-2][j] = Phi_new[nx-3][j];
    }
  for(int j=1; j<=ny-3; j++){
   Phi_new[nx-2][j]=Phi_new[nx-3][j];
  }

  for(int i=0; i<=nx-2; i++){
    Phi_new[i][0]=(-1)*Phi_new[i][1];
    Phi_new[i][ny-2]=(-1)*Phi_new[i][ny-3];
  }  
  
}


void paraview(string fileName, double **var, int nx, int ny, double dx, double dy){
  ofstream myfile;
  myfile.open(fileName);
  //------------------------------------------------------------//
    // Paraview header
  myfile << "# vtk DataFile Version 2.0\n";
  myfile << "FlowField\n";
  myfile << "ASCII\n";

    // Grid
  myfile << "DATASET STRUCTURED_GRID\n";
  myfile << "DIMENSIONS " << nx << " " << 1 << " " << ny << "\n";
  myfile << "POINTS " << nx*1*ny << " float\n";
  for(int j = 0; j <= ny-1; j++){
    for(int i = 0; i <= nx-1; i++){
      myfile << dx*i << " " << dy*j << " 0\n";
    }
  }
  
  // Data
  myfile << "\n";
  myfile << "POINT_DATA";
  myfile << " " << nx*ny << "\n";

  myfile << "\n";
  myfile << "SCALARS PHI float 1\n";
  myfile << "LOOKUP_TABLE default\n";
  for(int j = 0; j <= ny-1; j++){
    for(int i = 0; i <= nx-1; i++){
      myfile << var[i][j] << "\n";
    }
  }
  myfile.close();

}


int main(){

  int nx = 200;
  int ny = 80;
  int nn = 0;
  double Re = 300.;
  double dx = 4./real(ny-2);
  double dy = 1./real(ny-2);
  double dt = 0.01;
  string fileName;
  
  double **u;
  u = (double **) malloc ((nx-1) * sizeof(double));
  for(int i=0; i<nx-1; i++){
    u[i] = (double *) malloc((ny) * sizeof(double));
  }
  double **v;
  v = (double **) malloc ((nx) * sizeof(double));
  for(int i=0; i<nx; i++){
    v[i] = (double *) malloc((ny-1) * sizeof(double));
  }
  double **u_new;
  u_new = (double **) malloc ((nx-1) * sizeof(double));
  for(int i=0; i<nx-1; i++){
    u_new[i] = (double *) malloc((ny) * sizeof(double));
  }
  double **v_new;
  v_new = (double **) malloc ((nx) * sizeof(double));
  for(int i=0; i<nx; i++){
    v_new[i] = (double *) malloc((ny-1) * sizeof(double));
  }

  double **F;
  F = (double **) malloc ((nx-1) * sizeof(double));
  for(int i=0; i<nx-1; i++){
    F[i] = (double *) malloc((ny) * sizeof(double));
  }
  double **G;
  G = (double **) malloc ((nx) * sizeof(double));
  for(int i=0; i<nx; i++){
    G[i] = (double *) malloc((ny-1) * sizeof(double));
  }

  double **P;
  P = (double **) malloc ((nx) * sizeof(double));
  for(int i=0; i<nx; i++){
    P[i] = (double *) malloc((ny) * sizeof(double));
  }
  double **P_old;
  P_old = (double **) malloc ((nx) * sizeof(double));
  for(int i=0; i<nx; i++){
    P_old[i] = (double *) malloc((ny) * sizeof(double));
  }
  double **P_new;
  P_new = (double **) malloc ((nx) * sizeof(double));
  for(int i=0; i<nx; i++){
    P_new[i] = (double *) malloc((ny) * sizeof(double));
  }
  double **Phi;
  Phi = (double **) malloc ((nx-1) * sizeof(double));
  for(int i=0; i<nx-1; i++){
    Phi[i] = (double *) malloc((ny-1) * sizeof(double));
  }
  double **Phi_new;
  Phi_new = (double **) malloc ((nx-1) * sizeof(double));
  for(int i=0; i<nx-1; i++){
    Phi_new[i] = (double *) malloc((ny-1) * sizeof(double));
  }
  

  initialize(u,u_new,v,v_new,F,G,P,P_new,Phi,Phi_new,nx,ny);
  
  for(int n=1;n<=10000;n++){
    
    simulation_FG(u, u_new, v, v_new, F, G, nx, ny, Re, dx, dy, dt);
    simulation_p(u, u_new, v, v_new, F, G, P, P_new, nx, ny, Re, dx, dy, dt);
    simulation_uv(u, u_new, v, v_new, F, G, P, P_new, nx, ny, Re, dx, dy, dt);
    update(u,u_new,nx-1,ny);
    update(v,v_new,nx,ny-1);
    update(P,P_new,nx,ny);
    simulation_passiveScalar(u, u_new, v, v_new, F, G, P, P_new, Phi, Phi_new, nx, ny, Re, dx, dy, dt);
    update(Phi,Phi_new,nx-1,ny-1);
  
  
    cout << "n = " << n << "\n";
 
    if( n%10 == 0) {
      fileName = "phi_" + to_string(n) + ".vtk";
      paraview(fileName, Phi, nx-1, ny-1, dx, dy);
      fileName = "u_" + to_string(n) + ".vtk";
      paraview(fileName, u, nx-1, ny, dx, dy);
      fileName = "v_" + to_string(n) + ".vtk";
      paraview(fileName, v, nx, ny-1, dx, dy);
      fileName = "p_" + to_string(n) + ".vtk";
      paraview(fileName, P, nx, ny, dx, dy);
  
  //visualize(F,nx-1,ny);
  //visualize(G,nx,ny-1);
  //visualize(u,nx-1,ny);
  //visualize(v,nx,ny-1);
  //visualize(u_new,nx-1,ny);
  //visualize(v_new,nx,ny-1);
  //visualize(P,nx,ny);
  //visualize(P_new,nx,ny);
  //visualize(Phi,nx-1,ny-1);
  //visualize(Phi_new,nx-1,ny-1);
  }
 }
  
  
  
}
