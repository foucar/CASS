#include "helperfunctionsforSTDC.h"
#include <cmath>

//_______________________________________helper function that does a linear Regression__________________________________________
void linearRegression(const int nbrPoints, const double x[], const double y[], double &m, double &c)
{
    //--this funktion does a linear regression of 4 points--//
    //--getting a line that follows the form: y(x) = m*x + c--//
    //--return the x value for a given y(x) (yb)--//
    //-- => x = (y(x)-c)/m--//
    double SumXsq=0.,SumX=0.,SumY=0.,SumXY=0.;
    for (int i=0;i<nbrPoints;++i)
    {
        SumX    +=  x[i];
        SumY    +=  y[i];
        SumXY   += (x[i]*y[i]);
        SumXsq  += (x[i]*x[i]);
    }

    double a1 = ((SumX*SumX) - (nbrPoints*SumXsq));

    m = ((SumX*SumY) - (nbrPoints*SumXY)) / a1;
    c = ((SumX*SumXY) - (SumY*SumXsq)) / a1;
}
//_______________________________________helper function that does a weighted linear Regression__________________________________________
void gewichtetlinearRegression(const int nbrPoints, const double x[], const double y[], const double correctX, double &m, double &c)
{
    //--this funktion does a linear regression of 4 points--//
    //--getting a line that follows the form: y(x) = m*x + c--//
    //--return the x value for a given y(x) (yb)--//
    //-- => x = (y(x)-c)/m--//
    double SumXsq=0.,SumX=0.,SumY=0.,SumXY=0.,SumWeight=0.;
    for (int i=0;i<nbrPoints;++i)
    {
        double weight = (fabs(x[i]-correctX) > 1e-10) ? 1./fabs(x[i]-correctX): 100.;
        SumWeight += weight;
        SumX      += (x[i]*weight);
        SumY      += (y[i]*weight);
        SumXY     += (x[i]*y[i]*weight);
        SumXsq    += (x[i]*x[i]*weight);
    }

    double a1 = ((SumX*SumX) - (SumWeight*SumXsq));

    m = ((SumX*SumY) - (SumWeight*SumXY)) / a1;
    c = ((SumX*SumXY) - (SumY*SumXsq)) / a1;

}

//_________________________________create Newton Polynomial____________________________
void createNewtonPolynomial(const double * x, const double * y, double * coeff)
{
    //**this function creates the coefficients for Newton interpolating Polynomials  **//
    //**Newton Polynomial are Created from n Points and have the form                **//
    //**p(x) = c0 + c1(x-x0) + c2(x-x0)(x-x1)+...+c_(n-1)(x-x0)(x-x1)...(x-x_(n-2))  **//
    //**given that you have n Points (x0,y0), (x1,y1), ..., (x_(n-1),y_(n-1))        **//

    double f_x0_x1 = (y[1]-y[0]) / (x[1]-x[0]);
    double f_x1_x2 = (y[2]-y[1]) / (x[2]-x[1]);
    double f_x2_x3 = (y[3]-y[2]) / (x[3]-x[2]);

    double f_x0_x1_x2 = (f_x1_x2 - f_x0_x1) / (x[2]-x[0]);
    double f_x1_x2_x3 = (f_x2_x3 - f_x1_x2) / (x[3]-x[1]);

    double f_x0_x1_x2_x3 = (f_x1_x2_x3 - f_x0_x1_x2) / (x[3]-x[0]);

    coeff[0] = y[0];
    coeff[1] = f_x0_x1;
    coeff[2] = f_x0_x1_x2;
    coeff[3] = f_x0_x1_x2_x3;
}

//_________________________________evaluate Newton Polynomial____________________________
double evalNewtonPolynomial(const double * x, const double * coeff, double X)
{
    //**this function evaluates the Newton Polynomial that was created from n Points**//
    //** (x0,y0),..., (x(n-1),y(n-1)) with coefficients (c0,...,c(n-1))             **//
    //**using Horner's Rule                                                         **//

    double returnValue = coeff[3];
    returnValue = returnValue * (X - x[2]) + coeff[2];
    returnValue = returnValue * (X - x[1]) + coeff[1];
    returnValue = returnValue * (X - x[0]) + coeff[0];

    return returnValue;
}
//_________________________________Achims Numerical Approximation______________________
class MyPunkt
{
public: 
    MyPunkt(double xx, double yy):X(xx),Y(yy){}
    double &x() {return X;}
    double &y() {return Y;}
private:
    double X;
    double Y;
};
double findXForGivenY(const double * x, const double * coeff, const double Y, const double Start)
{
    //initialisiere die Grenzen//
    MyPunkt Low(x[1], evalNewtonPolynomial(x,coeff,x[1]));
    MyPunkt Up (x[2], evalNewtonPolynomial(x,coeff,x[2]));

    //initialisiere den iterierenden Punkt mit dem Startwert//
    MyPunkt p (Start, evalNewtonPolynomial(x,coeff,Start));

    //ist der Startpunkt schon der richtige Punkt//
    //liefere den dazugehoerigen x-Wert zurueck//
    if (p.y() == Y)
        return p.x();

    //finde heraus ob es ein positiver oder ein negativer Durchgang ist//
    bool Neg = (Low.y() > Up.y())?true:false;

    //der Startpunkt soll die richtige neue Grenze bilden//
    if (Neg)    //wenn es ein negativer Druchgang ist
    {
        if (p.y() > Y)      //ist der y-Wert groesser als der gewollte
            Low = p;        //bildet der Punkt die neue untere Grenze
        else if (p.y() < Y) //ist der y-Wert ist kleiner als der gewollte
            Up = p;         //bildet der Punkt die neue obere Grenze
        else                //ist der Punkt genau getroffen
            return p.x();   //liefer den dazugehoerigen x-Wert zurueck
    }
    else        //wenn es ein positiver Druchgang ist
    {
        if (p.y() > Y)      //und der y-Wert groesser als der gewollte
            Up = p;         //bildet der Punkt die neue obere Grenze
        else if (p.y() < Y) //und y-Wert ist kleiner als der gewollte
            Low = p;        //bildet der Punkt die neue untere Grenze
        else                //ist der Punkt genau getroffen
            return p.x();   //liefer den dazugehoerigen x-Wert zurueck
    }

    //iteriere solange bis der Abstand zwischen den x-Werten kleiner als 0.005
    while((Up.x()-Low.x()) > 0.005)
    {
        //bilde das arithmetische Mittel zwischen beiden Grenzen//
        //das ist der neue x-Wert unseres Punktes//
        p.x() = 0.5 * (Up.x()+Low.x());
        //finde den dazugehoerigen y-Wert//
        p.y() = evalNewtonPolynomial(x,coeff,p.x());

        if (Neg) //wenn es ein negativer Druchgang ist
        {
            if (p.y() > Y)      //und der y-Wert groesser als der gewollte
                Low = p;        //bildet der Punkt die neue untere Grenze
            else if (p.y() < Y) //und der y-Wert ist kleiner als der gewollte
                Up = p;         //bildet der Punkt die neue obere Grenze
            else                //ist der Punkt genau getroffen
                return p.x();   //liefer den dazugehoerigen x-Wert zurueck
        }
        else     //wenn es ein positiver Druchgang ist
        {
            if (p.y() > Y)      //und der y-Wert groesser als der gewollte
                Up = p;         //bildet der Punkt die neue obere Grenze
            else if (p.y() < Y) //und y-Wert ist kleiner als der gewollte
                Low = p;        //bildet der Punkt die neue untere Grenze
            else                //ist der Punkt genau getroffen
                return p.x();   //liefer den dazugehoerigen x-Wert zurueck
        }
//        std::cout<<"("<<Low.x<<","<<Low.y<<")   ("<<p.x<<","<<p.y<<")   ("<<Up.x<<","<<Up.y<<") "<<Y<<std::endl;
    }
    //ist der gewuenschte Abstand zwischen den x-Werten erreicht
    //liefere das arithmetische mittel zwischen beiden zurueck
    return ((Up.x() + Low.x())*0.5);
}
//_________________________________gib mir zurueck______________________
//double gmz(const double i, const double x0, const dvec MPuls)
//{
//    //calc the distance between the center of the Meanpuls(50) and the input puls(x0)//
//    double Abstand = x0 - 50.;
//
//    //calc the x position that is requested//
//    double x = i-Abstand;
//
//    //truncate this number and you know the bin left to the wanted x point//
//    int binLeft = (int) x;
//
//    //check wether we are still in range//
//    if (binLeft<0) return 0;
//    if (binLeft>=(MPuls.size()-1)) return 0;
//
//    //make a line through the leftbin and the right bin//
//    //give back the y for the asked x//
//    double m = MPuls[binLeft+1] - MPuls[binLeft];
//    double ret = m*(x-binLeft) + MPuls[binLeft];
//
//    return ret;
//}
//
