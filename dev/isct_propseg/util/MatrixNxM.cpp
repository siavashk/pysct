#include "MatrixNxM.h"
//#include <alglib/blas.h>
//#include <alglib/svd.h>
#include "linalg.h"
#include <vector>

using namespace std;
//using namespace ap;

/****************************************************************************
 * Fonction:	Matrice::Matrice
 * Description: Constructeur par défaut. 3X3 rempli de zero
 * Paramètres:	aucun
 * Retour:		aucun
 ****************************************************************************/
Matrice::Matrice()
{
	lignes_ = 3;
	colonnes_ = 3;
	auto all_zeros = std::vector<double>(0);
	matrice_.setcontent(lignes_, colonnes_, all_zeros.data());
}


/****************************************************************************
 * Fonction:	Matrice::Matrice
 * Description: Constructeur par paramètre ; rempli de zero
 * Paramètres:	- (unsigned int) ligne: nombre de lignes
 *				- (unsigned int) colonne: nombre de colonne
 * Retour:		aucun
 ****************************************************************************/
Matrice::Matrice(unsigned int lignes, unsigned int colonnes)
{
	lignes_ = lignes;
	colonnes_ = colonnes;
	auto all_zeros = std::vector<double>(lignes * colonnes);
	std::fill(all_zeros.begin(), all_zeros.end(), 0);
	matrice_.setcontent(lignes_, colonnes_, all_zeros.data());
}

/****************************************************************************
 * Fonction:	Matrice::Matrice
 * Description: Constructeur par parametre
 * Parametres:	- (unsigned int) ligne: nombre de lignes
 *				- (unsigned int) colonne: nombre de colonne
 * Retour:		aucun
 ****************************************************************************/
Matrice::Matrice(alglib::real_2d_array m, unsigned int lignes, unsigned int colonnes, bool newMatrice)
{
	lignes_ = lignes;
	colonnes_ = colonnes;
    if (newMatrice)
		matrice_.setcontent(lignes_, colonnes_, *m.c_ptr()->ptr.pp_double);
	else 
		matrice_ = m;
}


/****************************************************************************
 * Fonction:	Matrice::Matrice
 * Description: Constructeur par copie
 * Paramètres:	Matrice à copier
 * Retour:		aucun
 ****************************************************************************/
Matrice::Matrice(const Matrice& uneMatrice)
{
	lignes_ = uneMatrice.lignes_;
	colonnes_ = uneMatrice.colonnes_;
	matrice_.setcontent(lignes_, colonnes_, *uneMatrice.matrice_.c_ptr()->ptr.pp_double);
}


/****************************************************************************
 * Fonction:	Matrice::~Matrice
 * Description: Destructeur
 * Paramètres:	aucun
 * Retour:		aucun
 ****************************************************************************/
Matrice::~Matrice()
{
}


/****************************************************************************
 * Fonction:	Matrice::operateur()
 * Description: accéder à un element dans la matrice
 * Paramètres:	- (unsigned int) ligne: la position en ligne de l'element
 *				- (unsigned int) colonne: la position en colonne de l'element
 * Retour:		référence de l'élément
 ****************************************************************************/
double& Matrice::operator()(unsigned int ligne, unsigned int colonne)
{
	return matrice_(ligne,colonne);
}


/****************************************************************************
 * Fonction:	Matrice<T>::operateur=
 * Description: réalise la copie profonde de la matrice
 * Paramètres:	Matrice à copier
 * Retour:		aucun
 ****************************************************************************/
void Matrice::operator=(const Matrice& uneMatrice)
{
	lignes_ = uneMatrice.lignes_;
	colonnes_ = uneMatrice.colonnes_;
	matrice_.setcontent(lignes_, colonnes_, *uneMatrice.matrice_.c_ptr()->ptr.pp_double);
}


/****************************************************************************
 * Fonction:	Matrice::operateur*
 * Description: Multiplie la matrice avec celle passée en paramètre en vérifiant les dimensions
 * Paramètres:	Matrice à multiplier
 * Retour:		Matrice résultante
 ****************************************************************************/
Matrice Matrice::operator*(const Matrice& uneMatrice)
{
	Matrice result;
	if (uneMatrice.lignes_ == colonnes_) {
		result = Matrice(lignes_,uneMatrice.colonnes_);
        
		alglib::real_1d_array work;
		auto all_zeros = std::vector<double>(0);
		work.setcontent(lignes_, all_zeros.data());
        
        //matrixmatrixmultiply(matrice_,0,lignes_-1,0,colonnes_-1,0,uneMatrice.matrice_,false,uneMatrice.lignes_-1,0,uneMatrice.colonnes_-1,0,1.0,result.matrice_,0,lignes_-1,false,uneMatrice.colonnes_-1,1.0,work);
		//rmatrixgemm(lignes_,uneMatrice.colonnes_,colonnes_,1,matrice_,0,0,0,uneMatrice.matrice_,0,0,0,0,result.matrice_,0,0);
		for (size_t i = 0; i < lignes_; i++) {
			for (size_t j = 0; j < uneMatrice.colonnes_; j++) {
				for (size_t k = 0; k < colonnes_; k++)
					result.matrice_(i,j) += matrice_(i,k) * uneMatrice.matrice_(k,j);
			}
		}
	}
	else {
		cerr	<< "Error when multiplying matrices. The dimensions must match." << endl
				<< "(" << lignes_ << "," << colonnes_ << ") * (" << uneMatrice.lignes_ << "," << uneMatrice.colonnes_ << ")" << endl;
	}
	return result;
}


/****************************************************************************
 * Fonction:	Matrice::operateur+
 * Description: Additionne deux matrices. Il faut que les deux matrices soient de memes dimensions
 * Paramètres:	Matrice a additionner
 * Retour:		Matrice resultante
 ****************************************************************************/
Matrice Matrice::operator+(const Matrice& uneMatrice)
{
	Matrice result;
	if (uneMatrice.lignes_ == lignes_ && uneMatrice.colonnes_ == colonnes_) {
        
		result = Matrice(lignes_,colonnes_);
		for (size_t i=0; i<lignes_; i++) {
			for (size_t j=0; j<uneMatrice.colonnes_; j++)
				result.matrice_(i,j) = matrice_(i,j) + uneMatrice.matrice_(i,j);
		}
	}
	else {
		cerr	<< "Erreur lors de l'addition des matrices. Les dimensions doivent etre correctes." << endl
				<< "(" << lignes_ << "," << colonnes_ << ") + (" << uneMatrice.lignes_ << "," << uneMatrice.colonnes_ << ")" << endl;
	}
	return result;
}

/****************************************************************************
 * Fonction:	Matrice::operateur-
 * Description: Soustrait deux matrices. Il faut que les deux matrices soient de memes dimensions
 * Paramètres:	Matrice a soustraire
 * Retour:		Matrice resultante
 ****************************************************************************/
Matrice Matrice::operator-(const Matrice& uneMatrice)
{
	Matrice result;
	if (uneMatrice.lignes_ == lignes_ && uneMatrice.colonnes_ == colonnes_) {
        
		result = Matrice(lignes_,colonnes_);
		for (size_t i=0; i<lignes_; i++) {
			for (size_t j=0; j<uneMatrice.colonnes_; j++)
				result.matrice_(i,j) = matrice_(i,j) - uneMatrice.matrice_(i,j);
		}
	}
	else {
		cerr	<< "Erreur lors de la soustraction des matrices. Les dimensions doivent etre correctes." << endl
        << "(" << lignes_ << "," << colonnes_ << ") + (" << uneMatrice.lignes_ << "," << uneMatrice.colonnes_ << ")" << endl;
	}
	return result;
}


/****************************************************************************
 * Fonction:	Matrice::operateur/
 * Description: Divise tous les éléments de la matrice par une valeur réelle
 * Paramètres:	Valeur à diviser
 * Retour:		Matrice résultante
 ****************************************************************************/
Matrice Matrice::operator/(double valeur)
{
	Matrice result = Matrice(*this);
	for (size_t i=0; i<lignes_; i++) {
		for (size_t j=0; j<colonnes_; j++)
			result.matrice_(i,j) /= valeur;
	}
	return result;
}


/****************************************************************************
 * Fonction:	Matrice::transpose
 * Description: Calcule la matrice transposée
 * Paramètres:	Aucun
 * Retour:		Matrice résultante
 ****************************************************************************/
Matrice Matrice::transpose()
{
	Matrice result = Matrice(colonnes_,lignes_);
	for (size_t i=0; i<lignes_; i++) {
		for (size_t j=0; j<colonnes_; j++)
			result.matrice_(j,i) = matrice_(i,j);
	}
	return result;
}


/****************************************************************************
 * Fonction:	Matrice::norm
 * Description: Calcule la norme du vecteur. La matrice doit être un vecteur! Retourne 0.0 si n'est pas un vecteur.
 * Paramètres:	Aucun
 * Retour:		Norme du vecteur
 ****************************************************************************/
double Matrice::norm()
{
	double result = 0.0;
	if (lignes_ == 1) {
		for (unsigned int i=0; i<colonnes_; i++)
			result += matrice_(0,i) * matrice_(0,i);
	}
	else if (colonnes_ == 1) {
		for (unsigned int i=0; i<lignes_; i++)
			result += matrice_(i,0)*matrice_(i,0);
	}
	return sqrt(result);
}


/****************************************************************************
 * Fonction:	Matrice::operateur<<
 * Description: affiche la matrice
 * Paramètres:	matrice à afficher
 * Retour:		ostream
 ****************************************************************************/
ostream& operator<< (ostream& stream, const Matrice& uneMatrice) 
{
	for (unsigned int i=0; i<uneMatrice.lignes_; i++)
	{
		for (unsigned int j=0; j<uneMatrice.colonnes_; j++)
			stream << uneMatrice.matrice_(i,j) << "\t";
		stream << endl;
	}
	return stream;
}


/****************************************************************************
 * Fonction:	Matrice::operateur<<
 * Description: modifie la matrice
 * Paramètres:	matrice à modifier
 * Retour:		istream
 ****************************************************************************/
istream& operator>> (istream& stream, Matrice& uneMatrice) 
{
	for (unsigned int i=0; i<uneMatrice.lignes_; i++)
	{
		for (unsigned int j=0; j<uneMatrice.colonnes_; j++)
			stream >> uneMatrice.matrice_(i,j);
	}
	return stream;
}



Matrice Matrice::pinv(double tol)
{
    Matrice result;
    if (lignes_ > colonnes_)
        result = (*this).transpose().pinv(tol).transpose();
    else
    {
        alglib::real_1d_array w;
        alglib::real_2d_array u, vt;
        alglib::rmatrixsvd(matrice_, lignes_, colonnes_, 2, 2, 2, w, u, vt);
        
        double sum_s = 0.0;
        vector<double> s;
        int r = 0;
        for (size_t i=0; i<colonnes_; i++)
        {
            if (w(i) > tol) r++;
            s.push_back(w(i));
        }
        
        if (r == 0) {
            result = Matrice(lignes_,colonnes_);
            for (unsigned int x=0; x<lignes_; x++) {
                for (unsigned int y=0; y<colonnes_; y++) 
					result(x, y) = 0.0;
            }
        }
        else {
            Matrice S = Matrice(r,r), Um = Matrice(u,lignes_,r,true), Vtm = Matrice(vt,r,colonnes_,true);
            for (int k=0; k<r; k++) S(k,k) = 1.0/s[k];
            Matrice Um_t = Um.transpose(), Vtm_t = Vtm.transpose();
            result = Vtm_t*(S*Um_t);
        }
    }
    return result;
}
