// Estimate class contains attributes means and covs which are estimates of sample data matrices mean and covariance
// and nf and ncl as no of features and no of classes respectively

using namespace std;

class Estimate
{
	private:
	Matrix *means; // pointer to mean matrix to hold mean matrix for all classes
	Matrix *covs; // pointer to covariance matrix to hold covariance matrix for all classes
	int nf;
	int ncl;

	public:
	Estimate(); // default constructor
	
	Estimate(int,int); // parameterized constructor
	
	~Estimate(); // destructor
	
	Matrix * getMean(); // member function to access means attribute
	
	void setMean(Matrix *); // member function to set means attribute
	
	Matrix * getCovariance(); // member function to access covs attribute
	void setCovariance(Matrix *); // member function to set covs attibute

	// this was developed as alternate method to calculate estimates
	void calculateEstimates(const Matrix &train, int c); // function to calculate means and covariances for given sets of trained data
	
};
