#!/usr/bin/perl

print
"#define Sampler_OUTPUT_LEFT 0
#define Sampler_OUTPUT_RIGHT 1
";

$max_samples = 32;

for($i = 0; $ < $max_samples; $i++)
{
	print "#define LMS_PITCH_$i = " . (($i * 10) + 2);
	print "#define LMS_PITCH_$i = " . (($i * 10) + 2);
	print "#define LMS_PITCH_$i = " . (($i * 10) + 2);
	print "#define LMS_PITCH_$i = " . (($i * 10) + 2);
	print "#define LMS_PITCH_$i = " . (($i * 10) + 2);
	print "#define LMS_PITCH_$i = " . (($i * 10) + 2);
	print "#define LMS_PITCH_$i = " . (($i * 10) + 2);
	print "#define LMS_PITCH_$i = " . (($i * 10) + 2);
	print "#define LMS_PITCH_$i = " . (($i * 10) + 2);
	print "#define LMS_PITCH_$i = " . (($i * 10) + 2);
	print "#define LMS_PITCH_$i = " . (($i * 10) + 2);
	print "#define LMS_PITCH_$i = " . (($i * 10) + 2);
}

#define Sampler_Stereo_COUNT 7


print = "
#define Sampler_BASE_PITCH_MIN 0
// not 127, as we want 120/2 = 60 as the default:
#define Sampler_BASE_PITCH_MAX 120

//#define Sampler_RELEASE_MIN 0.001f
#define Sampler_RELEASE_MIN 0.01f
#define Sampler_RELEASE_MAX 2.0f

#define Sampler_NOTES 128
#define Sampler_FRAMES_MAX 1048576

/*Provide an arbitrary maximum number of samples the user can load*/
#define LMS_MAX_SAMPLE_COUNT 32

//#define Sampler_Stereo_LABEL "stereo_sampler"
#define Sampler_Stereo_LABEL "Euphoria"
";


