I generated these SLHA files in a rather hacky way. We know that the lifetime of the neutralino is a derived quantity from GMSB. Using the SPS8 definitions for the input parameters for the model, the remaining free parameters are lambda (cut-off scale in TeV) and the effective SUSY breaking scale.  This latter term can be parameterized by c_grav, which is a relation between the true and effective breaking scales, which in turn determines the mass of the gravitino. The relation of these two free parameters to the ctau is nearly linear:

c_grav(lambda) = alpha(lambda) * sqrt(ctau), where alpha(lambda) = a + b * lambda ^ c, such that alpha a scaling constant set for each lambda.

So, using this information, I first downloaded all of the SLHA files from the 2012 analysis via: https://github.com/kmcdermo/GMSB-MC2016-PRODUCTION
and performed a crude linear extrapolation that has since been replaced by this power law.

I then wrote a script to extract the c_grav, lambda, and width of the neutralino (i.e. the "true" lifetime of the particle from the width): extract_params.sh. The script outputs a file : width_cgrav.txt.

This file is read-in by: fitcgrav.C, which performs a linear regression for each lambda. Then, once all of the slopes are computed, I then derive the power law relation between the slope and the lambda.

After getting the terms out from the macro, I then run: gencgrav.C, which generates a text file for each lambda with the "new" c_grav's for the ctau's we wish to generate: Lamba${lamb}_cgrav.txt

This list of text files is then finally read in by: autogen_GMSB_isasugra.sh, which runs ISASUGRA from the ISAJET v 7.87 package. Since ISASUGRA is an interactive program, I had to make a funny use of echo to pass the parameters consistently.

This last script generates the SLHA files of the form: GMSB_Lambda${lambda}TeV_CTau${ctau}cm.slha
