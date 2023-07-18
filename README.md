# MaxLocJoin-EDP
MaxLocJoin-EDP is an edge-disjoint partitioning scheme designed to enable the independent evaluation of complex queries without inter-partition joins. This partitioning approach aims to improve query performance and reduce the need for costly join operations across partitions.

# Install Steps
System Requirement: 64-bit linux server with GCC. We have tested on linux server with CentOS 8.4 x86_64 where the version of GCC is 8.5.0.

You can compile MaxLocJoin-EDP in one command. Just run

``g++ -o mlj-edp main.cpp -std=c++11``

to compile the code and build executable 'mlj-edp'.

# Usage
MaxLocJoin-EDP is used to partition an RDF graph.

Use

``./mlj-edp rdf_triple_file_name output separator number_of_parts``

Now, MaxLocJoin-EDP only support RDF datasets in NT format. Each line is a triple of subject, property and object, where the tab "\t" or the space " " are used as the separator. If separator in the above command equals to "1", the separator is the space " "; if separator in the above command equals to "2", the separator is the tab "\t".

# Contact
For any questions or inquiries, please contact us at msd673@hnu.edu.cn.
