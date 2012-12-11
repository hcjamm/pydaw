/* 
 * File:   wavetables.h
 * Author: Jeff Hubbard
 * 
 * Wavetables for the wavetable oscillator.  Generated by the wavetable_generator tool in the tools directory
 *
 * Created on December 10, 2012, 7:01 PM
 */

#ifndef WAVETABLES_H
#define	WAVETABLES_H

#ifdef	__cplusplus
extern "C" {
#endif

    
#define plain_saw_count 1200

float plain_saw_array[plain_saw_count] = {
-0.988352f, -0.986702f, -0.985052f, -0.983402f, -0.981752f, -0.980102f, -0.978452f, -0.976802f, -0.975152f, -0.973502f, -0.971852f, -0.970202f, 
-0.968552f, -0.966902f, -0.965252f, -0.963602f, -0.961952f, -0.960302f, -0.958652f, -0.957002f, -0.955352f, -0.953702f, -0.952052f, -0.950402f, 
-0.948752f, -0.947102f, -0.945452f, -0.943802f, -0.942152f, -0.940502f, -0.938852f, -0.937202f, -0.935552f, -0.933902f, -0.932252f, -0.930602f, 
-0.928952f, -0.927302f, -0.925652f, -0.924002f, -0.922352f, -0.920702f, -0.919052f, -0.917402f, -0.915752f, -0.914102f, -0.912452f, -0.910802f, 
-0.909152f, -0.907502f, -0.905852f, -0.904202f, -0.902552f, -0.900902f, -0.899252f, -0.897602f, -0.895952f, -0.894302f, -0.892652f, -0.891002f, 
-0.889352f, -0.887702f, -0.886052f, -0.884402f, -0.882752f, -0.881102f, -0.879452f, -0.877802f, -0.876152f, -0.874502f, -0.872852f, -0.871202f, 
-0.869552f, -0.867902f, -0.866252f, -0.864602f, -0.862952f, -0.861302f, -0.859652f, -0.858002f, -0.856352f, -0.854702f, -0.853052f, -0.851402f, 
-0.849752f, -0.848102f, -0.846452f, -0.844802f, -0.843152f, -0.841502f, -0.839852f, -0.838202f, -0.836552f, -0.834902f, -0.833252f, -0.831602f, 
-0.829952f, -0.828302f, -0.826652f, -0.825002f, -0.823352f, -0.821702f, -0.820052f, -0.818402f, -0.816752f, -0.815102f, -0.813452f, -0.811802f, 
-0.810152f, -0.808502f, -0.806852f, -0.805202f, -0.803552f, -0.801902f, -0.800252f, -0.798602f, -0.796952f, -0.795302f, -0.793652f, -0.792002f, 
-0.790352f, -0.788702f, -0.787052f, -0.785402f, -0.783752f, -0.782102f, -0.780452f, -0.778802f, -0.777152f, -0.775502f, -0.773852f, -0.772202f, 
-0.770552f, -0.768902f, -0.767252f, -0.765602f, -0.763952f, -0.762302f, -0.760652f, -0.759002f, -0.757352f, -0.755702f, -0.754052f, -0.752402f, 
-0.750752f, -0.749102f, -0.747452f, -0.745802f, -0.744152f, -0.742501f, -0.740852f, -0.739202f, -0.737552f, -0.735902f, -0.734251f, -0.732602f, 
-0.730951f, -0.729302f, -0.727652f, -0.726002f, -0.724352f, -0.722701f, -0.721052f, -0.719401f, -0.717752f, -0.716102f, -0.714451f, -0.712802f, 
-0.711151f, -0.709502f, -0.707851f, -0.706201f, -0.704552f, -0.702901f, -0.701252f, -0.699601f, -0.697951f, -0.696301f, -0.694651f, -0.693002f, 
-0.691351f, -0.689701f, -0.688051f, -0.686401f, -0.684751f, -0.683101f, -0.681451f, -0.679801f, -0.678151f, -0.676501f, -0.674851f, -0.673201f, 
-0.671551f, -0.669901f, -0.668251f, -0.666601f, -0.664951f, -0.663301f, -0.661651f, -0.660001f, -0.658351f, -0.656701f, -0.655051f, -0.653401f, 
-0.651751f, -0.650101f, -0.648451f, -0.646801f, -0.645151f, -0.643501f, -0.641851f, -0.640201f, -0.638551f, -0.636901f, -0.635251f, -0.633601f, 
-0.631951f, -0.630301f, -0.628651f, -0.627001f, -0.625351f, -0.623701f, -0.622051f, -0.620401f, -0.618751f, -0.617101f, -0.615451f, -0.613801f, 
-0.612151f, -0.610501f, -0.608851f, -0.607201f, -0.605551f, -0.603901f, -0.602251f, -0.600601f, -0.598951f, -0.597301f, -0.595651f, -0.594001f, 
-0.592351f, -0.590701f, -0.589051f, -0.587401f, -0.585751f, -0.584101f, -0.582451f, -0.580801f, -0.579151f, -0.577501f, -0.575851f, -0.574201f, 
-0.572551f, -0.570901f, -0.569251f, -0.567601f, -0.565951f, -0.564301f, -0.562651f, -0.561001f, -0.559351f, -0.557701f, -0.556051f, -0.554401f, 
-0.552751f, -0.551101f, -0.549451f, -0.547801f, -0.546151f, -0.544501f, -0.542851f, -0.541201f, -0.539551f, -0.537901f, -0.536251f, -0.534601f, 
-0.532951f, -0.531301f, -0.529651f, -0.528001f, -0.526351f, -0.524701f, -0.523051f, -0.521401f, -0.519751f, -0.518101f, -0.516451f, -0.514801f, 
-0.513151f, -0.511501f, -0.509851f, -0.508201f, -0.506551f, -0.504901f, -0.503251f, -0.501601f, -0.499951f, -0.498301f, -0.496651f, -0.495001f, 
-0.493351f, -0.491701f, -0.490051f, -0.488401f, -0.486751f, -0.485101f, -0.483451f, -0.481801f, -0.480151f, -0.478501f, -0.476851f, -0.475201f, 
-0.473551f, -0.471901f, -0.470251f, -0.468601f, -0.466951f, -0.465301f, -0.463651f, -0.462001f, -0.460351f, -0.458701f, -0.457051f, -0.455401f, 
-0.453751f, -0.452101f, -0.450451f, -0.448801f, -0.447151f, -0.445501f, -0.443851f, -0.442201f, -0.440551f, -0.438901f, -0.437251f, -0.435601f, 
-0.433951f, -0.432301f, -0.430651f, -0.429001f, -0.427351f, -0.425701f, -0.424051f, -0.422401f, -0.420751f, -0.419101f, -0.417451f, -0.415801f, 
-0.414151f, -0.412501f, -0.410851f, -0.409201f, -0.407551f, -0.405901f, -0.404251f, -0.402601f, -0.400951f, -0.399301f, -0.397651f, -0.396001f, 
-0.394351f, -0.392701f, -0.391051f, -0.389401f, -0.387751f, -0.386101f, -0.384451f, -0.382801f, -0.381151f, -0.379501f, -0.377851f, -0.376201f, 
-0.374551f, -0.372901f, -0.371251f, -0.369601f, -0.367951f, -0.366301f, -0.364651f, -0.363001f, -0.361351f, -0.359701f, -0.358051f, -0.356401f, 
-0.354751f, -0.353101f, -0.351451f, -0.349801f, -0.348151f, -0.346501f, -0.344851f, -0.343201f, -0.341551f, -0.339901f, -0.338251f, -0.336601f, 
-0.334951f, -0.333301f, -0.331651f, -0.330001f, -0.328351f, -0.326701f, -0.325051f, -0.323401f, -0.321751f, -0.320101f, -0.318451f, -0.316801f, 
-0.315151f, -0.313501f, -0.311851f, -0.310201f, -0.308551f, -0.306901f, -0.305251f, -0.303601f, -0.301951f, -0.300301f, -0.298651f, -0.297001f, 
-0.295351f, -0.293701f, -0.292051f, -0.290401f, -0.288751f, -0.287101f, -0.285451f, -0.283801f, -0.282151f, -0.280501f, -0.278851f, -0.277201f, 
-0.275551f, -0.273901f, -0.272251f, -0.270601f, -0.268951f, -0.267301f, -0.265651f, -0.264001f, -0.262351f, -0.260701f, -0.259051f, -0.257401f, 
-0.255751f, -0.254101f, -0.252451f, -0.250801f, -0.249151f, -0.247501f, -0.245851f, -0.244201f, -0.242551f, -0.240901f, -0.239251f, -0.237601f, 
-0.235951f, -0.234301f, -0.232651f, -0.231001f, -0.229351f, -0.227701f, -0.226051f, -0.224401f, -0.222751f, -0.221101f, -0.219451f, -0.217801f, 
-0.216151f, -0.214501f, -0.212851f, -0.211201f, -0.209551f, -0.207901f, -0.206251f, -0.204601f, -0.202951f, -0.201301f, -0.199651f, -0.198001f, 
-0.196351f, -0.194701f, -0.193051f, -0.191401f, -0.189751f, -0.188101f, -0.186451f, -0.184801f, -0.183151f, -0.181501f, -0.179851f, -0.178201f, 
-0.176551f, -0.174901f, -0.173251f, -0.171601f, -0.169951f, -0.168301f, -0.166651f, -0.165001f, -0.163351f, -0.161701f, -0.160051f, -0.158401f, 
-0.156751f, -0.155101f, -0.153451f, -0.151801f, -0.150151f, -0.148501f, -0.146851f, -0.145201f, -0.143551f, -0.141901f, -0.140251f, -0.138601f, 
-0.136951f, -0.135301f, -0.133651f, -0.132001f, -0.130351f, -0.128701f, -0.127051f, -0.125401f, -0.123751f, -0.122101f, -0.120451f, -0.118801f, 
-0.117151f, -0.115501f, -0.113851f, -0.112201f, -0.110551f, -0.108901f, -0.107251f, -0.105601f, -0.103951f, -0.102301f, -0.100651f, -0.099001f, 
-0.097351f, -0.095701f, -0.094051f, -0.092401f, -0.090751f, -0.089101f, -0.087451f, -0.085801f, -0.084151f, -0.082501f, -0.080851f, -0.079201f, 
-0.077551f, -0.075901f, -0.074251f, -0.072601f, -0.070951f, -0.069301f, -0.067651f, -0.066001f, -0.064351f, -0.062701f, -0.061051f, -0.059401f, 
-0.057751f, -0.056101f, -0.054451f, -0.052801f, -0.051151f, -0.049501f, -0.047851f, -0.046201f, -0.044551f, -0.042901f, -0.041251f, -0.039601f, 
-0.037951f, -0.036301f, -0.034651f, -0.033001f, -0.031351f, -0.029701f, -0.028051f, -0.026401f, -0.024751f, -0.023101f, -0.021451f, -0.019801f, 
-0.018151f, -0.016501f, -0.014851f, -0.013201f, -0.011551f, -0.009901f, -0.008251f, -0.006601f, -0.004951f, -0.003301f, -0.001651f, -0.000001f, 
0.001649f, 0.003299f, 0.004949f, 0.006599f, 0.008249f, 0.009899f, 0.011549f, 0.013199f, 0.014849f, 0.016499f, 0.018149f, 0.019799f, 
0.021449f, 0.023099f, 0.024749f, 0.026399f, 0.028049f, 0.029699f, 0.031349f, 0.032999f, 0.034649f, 0.036299f, 0.037949f, 0.039599f, 
0.041249f, 0.042899f, 0.044549f, 0.046199f, 0.047849f, 0.049499f, 0.051149f, 0.052799f, 0.054449f, 0.056099f, 0.057749f, 0.059399f, 
0.061049f, 0.062699f, 0.064349f, 0.065999f, 0.067649f, 0.069299f, 0.070949f, 0.072599f, 0.074249f, 0.075899f, 0.077549f, 0.079199f, 
0.080849f, 0.082499f, 0.084149f, 0.085799f, 0.087449f, 0.089099f, 0.090749f, 0.092399f, 0.094049f, 0.095699f, 0.097349f, 0.098999f, 
0.100649f, 0.102299f, 0.103949f, 0.105599f, 0.107249f, 0.108899f, 0.110549f, 0.112199f, 0.113849f, 0.115499f, 0.117149f, 0.118799f, 
0.120449f, 0.122099f, 0.123749f, 0.125399f, 0.127049f, 0.128699f, 0.130349f, 0.131999f, 0.133649f, 0.135299f, 0.136949f, 0.138599f, 
0.140249f, 0.141899f, 0.143549f, 0.145199f, 0.146849f, 0.148499f, 0.150149f, 0.151799f, 0.153449f, 0.155099f, 0.156749f, 0.158399f, 
0.160049f, 0.161699f, 0.163349f, 0.164999f, 0.166649f, 0.168299f, 0.169949f, 0.171599f, 0.173249f, 0.174899f, 0.176549f, 0.178199f, 
0.179849f, 0.181499f, 0.183149f, 0.184799f, 0.186449f, 0.188099f, 0.189749f, 0.191399f, 0.193049f, 0.194699f, 0.196349f, 0.197999f, 
0.199649f, 0.201299f, 0.202949f, 0.204599f, 0.206249f, 0.207899f, 0.209549f, 0.211199f, 0.212849f, 0.214499f, 0.216149f, 0.217799f, 
0.219449f, 0.221099f, 0.222749f, 0.224399f, 0.226049f, 0.227699f, 0.229349f, 0.230999f, 0.232649f, 0.234299f, 0.235949f, 0.237599f, 
0.239249f, 0.240899f, 0.242549f, 0.244199f, 0.245849f, 0.247499f, 0.249149f, 0.250799f, 0.252449f, 0.254099f, 0.255749f, 0.257399f, 
0.259049f, 0.260699f, 0.262349f, 0.263999f, 0.265649f, 0.267299f, 0.268949f, 0.270599f, 0.272249f, 0.273899f, 0.275549f, 0.277199f, 
0.278849f, 0.280499f, 0.282149f, 0.283799f, 0.285449f, 0.287099f, 0.288749f, 0.290399f, 0.292049f, 0.293699f, 0.295349f, 0.296999f, 
0.298649f, 0.300299f, 0.301949f, 0.303599f, 0.305249f, 0.306899f, 0.308549f, 0.310199f, 0.311849f, 0.313499f, 0.315149f, 0.316799f, 
0.318449f, 0.320099f, 0.321749f, 0.323399f, 0.325049f, 0.326699f, 0.328349f, 0.329999f, 0.331649f, 0.333299f, 0.334949f, 0.336599f, 
0.338249f, 0.339899f, 0.341549f, 0.343199f, 0.344849f, 0.346499f, 0.348149f, 0.349799f, 0.351449f, 0.353099f, 0.354749f, 0.356399f, 
0.358049f, 0.359699f, 0.361349f, 0.362999f, 0.364649f, 0.366299f, 0.367949f, 0.369599f, 0.371249f, 0.372900f, 0.374550f, 0.376199f, 
0.377849f, 0.379499f, 0.381150f, 0.382800f, 0.384450f, 0.386100f, 0.387750f, 0.389399f, 0.391050f, 0.392700f, 0.394350f, 0.396000f, 
0.397650f, 0.399300f, 0.400950f, 0.402600f, 0.404250f, 0.405900f, 0.407550f, 0.409200f, 0.410850f, 0.412500f, 0.414150f, 0.415800f, 
0.417450f, 0.419100f, 0.420750f, 0.422400f, 0.424050f, 0.425700f, 0.427350f, 0.429000f, 0.430650f, 0.432300f, 0.433950f, 0.435600f, 
0.437250f, 0.438900f, 0.440550f, 0.442200f, 0.443850f, 0.445500f, 0.447150f, 0.448800f, 0.450450f, 0.452100f, 0.453750f, 0.455400f, 
0.457050f, 0.458700f, 0.460350f, 0.462000f, 0.463650f, 0.465300f, 0.466950f, 0.468600f, 0.470250f, 0.471900f, 0.473550f, 0.475200f, 
0.476850f, 0.478500f, 0.480150f, 0.481800f, 0.483450f, 0.485100f, 0.486750f, 0.488400f, 0.490050f, 0.491700f, 0.493350f, 0.495000f, 
0.496650f, 0.498300f, 0.499950f, 0.501600f, 0.503250f, 0.504900f, 0.506550f, 0.508200f, 0.509850f, 0.511500f, 0.513150f, 0.514800f, 
0.516450f, 0.518100f, 0.519750f, 0.521400f, 0.523050f, 0.524700f, 0.526350f, 0.528000f, 0.529650f, 0.531300f, 0.532950f, 0.534600f, 
0.536250f, 0.537900f, 0.539550f, 0.541200f, 0.542850f, 0.544500f, 0.546150f, 0.547800f, 0.549450f, 0.551100f, 0.552750f, 0.554400f, 
0.556050f, 0.557700f, 0.559350f, 0.561000f, 0.562650f, 0.564300f, 0.565950f, 0.567600f, 0.569250f, 0.570900f, 0.572550f, 0.574200f, 
0.575850f, 0.577500f, 0.579150f, 0.580800f, 0.582450f, 0.584100f, 0.585750f, 0.587400f, 0.589050f, 0.590700f, 0.592350f, 0.594000f, 
0.595650f, 0.597300f, 0.598950f, 0.600600f, 0.602250f, 0.603900f, 0.605550f, 0.607200f, 0.608850f, 0.610500f, 0.612150f, 0.613800f, 
0.615450f, 0.617100f, 0.618750f, 0.620400f, 0.622050f, 0.623700f, 0.625350f, 0.627000f, 0.628650f, 0.630300f, 0.631950f, 0.633600f, 
0.635250f, 0.636900f, 0.638550f, 0.640200f, 0.641850f, 0.643500f, 0.645150f, 0.646800f, 0.648450f, 0.650100f, 0.651750f, 0.653400f, 
0.655050f, 0.656700f, 0.658350f, 0.660000f, 0.661650f, 0.663300f, 0.664950f, 0.666600f, 0.668250f, 0.669900f, 0.671550f, 0.673200f, 
0.674850f, 0.676500f, 0.678150f, 0.679800f, 0.681450f, 0.683100f, 0.684750f, 0.686400f, 0.688050f, 0.689700f, 0.691350f, 0.693000f, 
0.694650f, 0.696300f, 0.697950f, 0.699600f, 0.701250f, 0.702900f, 0.704550f, 0.706200f, 0.707850f, 0.709500f, 0.711150f, 0.712800f, 
0.714450f, 0.716100f, 0.717750f, 0.719400f, 0.721050f, 0.722700f, 0.724350f, 0.726000f, 0.727650f, 0.729300f, 0.730950f, 0.732600f, 
0.734250f, 0.735900f, 0.737550f, 0.739200f, 0.740850f, 0.742500f, 0.744150f, 0.745800f, 0.747450f, 0.749100f, 0.750750f, 0.752400f, 
0.754050f, 0.755700f, 0.757350f, 0.759000f, 0.760650f, 0.762300f, 0.763950f, 0.765600f, 0.767250f, 0.768900f, 0.770550f, 0.772200f, 
0.773850f, 0.775500f, 0.777150f, 0.778800f, 0.780450f, 0.782100f, 0.783750f, 0.785400f, 0.787050f, 0.788700f, 0.790350f, 0.792000f, 
0.793650f, 0.795300f, 0.796950f, 0.798600f, 0.800250f, 0.801900f, 0.803550f, 0.805200f, 0.806850f, 0.808500f, 0.810150f, 0.811800f, 
0.813450f, 0.815100f, 0.816750f, 0.818400f, 0.820050f, 0.821700f, 0.823350f, 0.825000f, 0.826650f, 0.828300f, 0.829950f, 0.831600f, 
0.833250f, 0.834900f, 0.836550f, 0.838200f, 0.839850f, 0.841500f, 0.843150f, 0.844800f, 0.846450f, 0.848100f, 0.849750f, 0.851400f, 
0.853050f, 0.854700f, 0.856350f, 0.858000f, 0.859650f, 0.861300f, 0.862950f, 0.864600f, 0.866250f, 0.867900f, 0.869550f, 0.871200f, 
0.872850f, 0.874500f, 0.876150f, 0.877800f, 0.879450f, 0.881100f, 0.882750f, 0.884400f, 0.886050f, 0.887700f, 0.889350f, 0.891000f, 
0.892650f, 0.894300f, 0.895950f, 0.897600f, 0.899250f, 0.900900f, 0.902550f, 0.904200f, 0.905850f, 0.907500f, 0.909150f, 0.910800f, 
0.912450f, 0.914100f, 0.915750f, 0.917400f, 0.919050f, 0.920700f, 0.922350f, 0.924000f, 0.925650f, 0.927300f, 0.928950f, 0.930600f, 
0.932250f, 0.933900f, 0.935550f, 0.937200f, 0.938850f, 0.940500f, 0.942150f, 0.943800f, 0.945450f, 0.947100f, 0.948750f, 0.950400f, 
0.952050f, 0.953700f, 0.955350f, 0.957000f, 0.958650f, 0.960300f, 0.961950f, 0.963600f, 0.965250f, 0.966900f, 0.968550f, 0.970200f, 
0.971850f, 0.973500f, 0.975150f, 0.976800f, 0.978450f, 0.980100f, 0.981750f, 0.983400f, 0.985050f, 0.986700f, 0.988350f, 0.990000f, 
};


#define superbsaw_count 1200

float superbsaw_array[superbsaw_count] = {
-0.785764f, -0.784297f, -0.782809f, -0.781301f, -0.779772f, -0.778223f, -0.776654f, -0.775065f, -0.773455f, -0.771825f, -0.770176f, -0.768506f, 
-0.766816f, -0.765107f, -0.763378f, -0.761629f, -0.759860f, -0.758072f, -0.756264f, -0.754437f, -0.752590f, -0.750724f, -0.748839f, -0.746934f, 
-0.745010f, -0.743067f, -0.741105f, -0.739124f, -0.737124f, -0.735105f, -0.733067f, -0.731011f, -0.728935f, -0.726842f, -0.724729f, -0.722599f, 
-0.720450f, -0.718282f, -0.716096f, -0.713893f, -0.711670f, -0.709430f, -0.707172f, -0.704896f, -0.702602f, -0.700291f, -0.697962f, -0.695615f, 
-0.693250f, -0.690868f, -0.688469f, -0.686052f, -0.683619f, -0.681168f, -0.678700f, -0.676214f, -0.673712f, -0.671194f, -0.668658f, -0.666106f, 
-0.663537f, -0.660951f, -0.658349f, -0.655731f, -0.653096f, -0.650445f, -0.647778f, -0.645095f, -0.642396f, -0.639681f, -0.636951f, -0.634204f, 
-0.631442f, -0.628664f, -0.625871f, -0.623063f, -0.620239f, -0.617400f, -0.614546f, -0.611677f, -0.608793f, -0.605894f, -0.602980f, -0.600052f, 
-0.597108f, -0.594151f, -0.591179f, -0.588193f, -0.585192f, -0.582177f, -0.579148f, -0.576106f, -0.573049f, -0.569979f, -0.566894f, -0.563797f, 
-0.560685f, -0.557561f, -0.554423f, -0.551271f, -0.548107f, -0.544929f, -0.541739f, -0.538536f, -0.535320f, -0.532091f, -0.528849f, -0.525596f, 
-0.522329f, -0.519051f, -0.515760f, -0.512457f, -0.509143f, -0.505816f, -0.502477f, -0.499127f, -0.495765f, -0.492392f, -0.489007f, -0.485611f, 
-0.482204f, -0.478785f, -0.475356f, -0.471915f, -0.468464f, -0.465002f, -0.461530f, -0.458046f, -0.454553f, -0.451049f, -0.447535f, -0.444011f, 
-0.440477f, -0.436933f, -0.433379f, -0.429815f, -0.426242f, -0.422659f, -0.419067f, -0.415466f, -0.411855f, -0.408236f, -0.404607f, -0.400969f, 
-0.397323f, -0.393668f, -0.390005f, -0.386333f, -0.382652f, -0.378963f, -0.375267f, -0.371562f, -0.367849f, -0.364128f, -0.360400f, -0.356664f, 
-0.352921f, -0.349170f, -0.345412f, -0.341646f, -0.337874f, -0.334094f, -0.330308f, -0.326515f, -0.322715f, -0.318909f, -0.315096f, -0.311277f, 
-0.307451f, -0.303620f, -0.299782f, -0.295939f, -0.292090f, -0.288235f, -0.284374f, -0.280508f, -0.276637f, -0.272760f, -0.268879f, -0.264992f, 
-0.261100f, -0.257204f, -0.253302f, -0.249396f, -0.245486f, -0.241571f, -0.237652f, -0.233729f, -0.229802f, -0.225870f, -0.221935f, -0.217996f, 
-0.214054f, -0.210108f, -0.206158f, -0.202206f, -0.198250f, -0.194291f, -0.190329f, -0.186364f, -0.182396f, -0.178426f, -0.174453f, -0.170478f, 
-0.166501f, -0.162521f, -0.158539f, -0.154556f, -0.150570f, -0.146582f, -0.142593f, -0.138603f, -0.134611f, -0.130617f, -0.126623f, -0.122627f, 
-0.118630f, -0.114633f, -0.110634f, -0.106635f, -0.102635f, -0.098635f, -0.094635f, -0.090634f, -0.086633f, -0.082632f, -0.078632f, -0.074631f, 
-0.070631f, -0.066631f, -0.062632f, -0.058633f, -0.054635f, -0.050638f, -0.046642f, -0.042647f, -0.038653f, -0.034660f, -0.030669f, -0.026679f, 
-0.022691f, -0.018704f, -0.014720f, -0.010737f, -0.006756f, -0.002778f, 0.001199f, 0.005172f, 0.009144f, 0.013113f, 0.017079f, 0.021043f, 
0.025003f, 0.028961f, 0.032915f, 0.036867f, 0.040815f, 0.044759f, 0.048700f, 0.052638f, 0.056571f, 0.060501f, 0.064427f, 0.068349f, 
0.072267f, 0.076180f, 0.080090f, 0.083994f, 0.087894f, 0.091790f, 0.095681f, 0.099566f, 0.103447f, 0.107323f, 0.111194f, 0.115059f, 
0.118919f, 0.122773f, 0.126622f, 0.130465f, 0.134303f, 0.138134f, 0.141960f, 0.145779f, 0.149593f, 0.153399f, 0.157200f, 0.160994f, 
0.164782f, 0.168562f, 0.172336f, 0.176103f, 0.179864f, 0.183617f, 0.187363f, 0.191101f, 0.194832f, 0.198556f, 0.202272f, 0.205981f, 
0.209681f, 0.213374f, 0.217059f, 0.220736f, 0.224405f, 0.228065f, 0.231718f, 0.235361f, 0.238997f, 0.242623f, 0.246241f, 0.249850f, 
0.253450f, 0.257041f, 0.260624f, 0.264196f, 0.267760f, 0.271314f, 0.274859f, 0.278395f, 0.281920f, 0.285436f, 0.288942f, 0.292439f, 
0.295925f, 0.299401f, 0.302867f, 0.306323f, 0.309768f, 0.313203f, 0.316628f, 0.320042f, 0.323445f, 0.326837f, 0.330219f, 0.333590f, 
0.336949f, 0.340298f, 0.343635f, 0.346961f, 0.350275f, 0.353578f, 0.356870f, 0.360150f, 0.363418f, 0.366675f, 0.369919f, 0.373152f, 
0.376373f, 0.379581f, 0.382778f, 0.385962f, 0.389133f, 0.392293f, 0.395439f, 0.398574f, 0.401695f, 0.404804f, 0.407900f, 0.410983f, 
0.414053f, 0.417110f, 0.420154f, 0.423185f, 0.426202f, 0.429206f, 0.432197f, 0.435174f, 0.438138f, 0.441088f, 0.444024f, 0.446947f, 
0.449856f, 0.452750f, 0.455631f, 0.458498f, 0.461350f, 0.464189f, 0.467013f, 0.469822f, 0.472618f, 0.475399f, 0.478165f, 0.480916f, 
0.483653f, 0.486376f, 0.489083f, 0.491776f, 0.494453f, 0.497116f, 0.499764f, 0.502396f, 0.505013f, 0.507615f, 0.510202f, 0.512773f, 
0.515329f, 0.517869f, 0.520394f, 0.522903f, 0.525397f, 0.527875f, 0.530337f, 0.532783f, 0.535213f, 0.537627f, 0.540025f, 0.542408f, 
0.544774f, 0.547123f, 0.549457f, 0.551774f, 0.554075f, 0.556360f, 0.558628f, 0.560879f, 0.563114f, 0.565333f, 0.567534f, 0.569719f, 
0.571888f, 0.574039f, 0.576174f, 0.578291f, 0.580392f, 0.582476f, 0.584542f, 0.586592f, 0.588624f, 0.590640f, 0.592638f, 0.594618f, 
0.596582f, 0.598528f, 0.600456f, 0.602368f, 0.604261f, 0.606138f, 0.607996f, 0.609837f, 0.611661f, 0.613466f, 0.615254f, 0.617024f, 
0.618777f, 0.620511f, 0.622228f, 0.623927f, 0.625607f, 0.627270f, 0.628915f, 0.630542f, 0.632150f, 0.633741f, 0.635313f, 0.636867f, 
0.638403f, 0.639921f, 0.641420f, 0.642901f, 0.644364f, 0.645808f, 0.647234f, 0.648642f, 0.650031f, 0.651402f, 0.652754f, 0.654087f, 
0.655402f, 0.656699f, 0.657977f, 0.659236f, 0.660476f, 0.661698f, 0.662901f, 0.664086f, 0.665251f, 0.666398f, 0.667527f, 0.668636f, 
0.669726f, 0.670798f, 0.671851f, 0.672885f, 0.673900f, 0.674896f, 0.675873f, 0.676832f, 0.677771f, 0.678691f, 0.679593f, 0.680475f, 
0.681338f, 0.682183f, 0.683008f, 0.683814f, 0.684601f, 0.685369f, 0.686119f, 0.686849f, 0.687559f, 0.688251f, 0.688924f, 0.689577f, 
0.690212f, 0.690827f, 0.691423f, 0.692000f, 0.692558f, 0.693097f, 0.693616f, 0.694116f, 0.694598f, 0.695060f, 0.695503f, 0.695926f, 
0.696331f, 0.696716f, 0.697082f, 0.697429f, 0.697757f, 0.698066f, 0.698356f, 0.698626f, 0.698877f, 0.699110f, 0.699323f, 0.699516f, 
0.699691f, 0.699847f, 0.699983f, 0.700101f, 0.700199f, 0.700278f, 0.700338f, 0.700379f, 0.700401f, 0.700404f, 0.700388f, 0.700353f, 
0.700299f, 0.700226f, 0.700134f, 0.700022f, 0.699892f, 0.699743f, 0.699575f, 0.699388f, 0.699183f, 0.698958f, 0.698714f, 0.698452f, 
0.698171f, 0.697871f, 0.697552f, 0.697214f, 0.696858f, 0.696483f, 0.696089f, 0.695676f, 0.695245f, 0.694795f, 0.694327f, 0.693840f, 
0.693334f, 0.692810f, 0.692267f, 0.691706f, 0.691126f, 0.690528f, 0.689911f, 0.689277f, 0.688623f, 0.687951f, 0.687261f, 0.686553f, 
0.685827f, 0.685082f, 0.684319f, 0.683538f, 0.682739f, 0.681922f, 0.681086f, 0.680233f, 0.679361f, 0.678472f, 0.677565f, 0.676640f, 
0.675697f, 0.674736f, 0.673757f, 0.672761f, 0.671747f, 0.670715f, 0.669666f, 0.668599f, 0.667514f, 0.666412f, 0.665293f, 0.990000f, 
0.228344f, 0.227429f, 0.226508f, 0.225582f, 0.224650f, 0.223712f, 0.222769f, 0.221820f, 0.220866f, 0.219907f, 0.218942f, 0.217971f, 
0.216996f, 0.216015f, 0.215028f, 0.214037f, 0.213040f, 0.212038f, 0.211030f, 0.210018f, 0.209001f, 0.207978f, 0.206950f, 0.205918f, 
0.204880f, 0.203837f, 0.202790f, 0.201737f, 0.200680f, 0.199618f, 0.198551f, 0.197479f, 0.196403f, 0.195321f, 0.194235f, 0.193145f, 
0.192050f, 0.190950f, 0.189846f, 0.188737f, 0.187624f, 0.186506f, 0.185384f, 0.184257f, 0.183126f, 0.181991f, 0.180852f, 0.179708f, 
0.178560f, 0.177408f, 0.176251f, 0.175091f, 0.173927f, 0.172758f, 0.171585f, 0.170409f, 0.169228f, 0.168044f, 0.166856f, 0.165664f, 
0.164468f, 0.163268f, 0.162064f, 0.160857f, 0.159647f, 0.158432f, 0.157214f, 0.155992f, 0.154767f, 0.153539f, 0.152307f, 0.151071f, 
0.149832f, 0.148590f, 0.147344f, 0.146096f, 0.144844f, 0.143588f, 0.142330f, 0.141068f, 0.139803f, 0.138536f, 0.137265f, 0.135991f, 
0.134714f, 0.133435f, 0.132152f, 0.130867f, 0.129579f, 0.128288f, 0.126994f, 0.125698f, 0.124399f, 0.123097f, 0.121793f, 0.120486f, 
0.119177f, 0.117865f, 0.116551f, 0.115234f, 0.113915f, 0.112594f, 0.111270f, 0.109944f, 0.108616f, 0.107286f, 0.105954f, 0.104619f, 
0.103283f, 0.101944f, 0.100603f, 0.099261f, 0.097916f, 0.096570f, 0.095222f, 0.093872f, 0.092520f, 0.091167f, 0.089812f, 0.088455f, 
0.087096f, 0.085736f, 0.084375f, 0.083012f, 0.081647f, 0.080282f, 0.078914f, 0.077546f, 0.076176f, 0.074804f, 0.073432f, 0.072058f, 
0.070684f, 0.069308f, 0.067931f, 0.066553f, 0.065173f, 0.063793f, 0.062413f, 0.061031f, 0.059648f, 0.058264f, 0.056880f, 0.055495f, 
0.054109f, 0.052723f, 0.051336f, 0.049948f, 0.048560f, 0.047172f, 0.045782f, 0.044393f, 0.043003f, 0.041613f, 0.040222f, 0.038831f, 
0.037440f, 0.036049f, 0.034657f, 0.033265f, 0.031874f, 0.030482f, 0.029090f, 0.027698f, 0.026306f, 0.024915f, 0.023523f, 0.022132f, 
0.020741f, 0.019350f, 0.017959f, 0.016569f, 0.015179f, 0.013789f, 0.012400f, 0.011012f, 0.009624f, 0.008236f, 0.006849f, 0.005463f, 
0.004077f, 0.002692f, 0.001308f, -0.000076f, -0.001458f, -0.002840f, -0.004221f, -0.005601f, -0.006980f, -0.008358f, -0.009735f, -0.011111f, 
-0.012486f, -0.013860f, -0.015233f, -0.016604f, -0.017974f, -0.019343f, -0.020710f, -0.022076f, -0.023441f, -0.024804f, -0.026166f, -0.027527f, 
-0.028885f, -0.030242f, -0.031598f, -0.032952f, -0.034304f, -0.035655f, -0.037004f, -0.038350f, -0.039696f, -0.041039f, -0.042380f, -0.043720f, 
-0.045057f, -0.046393f, -0.047726f, -0.049057f, -0.050387f, -0.051714f, -0.053039f, -0.054361f, -0.055682f, -0.057000f, -0.058316f, -0.059629f, 
-0.060941f, -0.062249f, -0.063555f, -0.064859f, -0.066160f, -0.067459f, -0.068755f, -0.070049f, -0.071339f, -0.072628f, -0.073913f, -0.075196f, 
-0.076475f, -0.077752f, -0.079026f, -0.080298f, -0.081566f, -0.082831f, -0.084094f, -0.085353f, -0.086609f, -0.087863f, -0.089113f, -0.090360f, 
-0.091603f, -0.092844f, -0.094081f, -0.095315f, -0.096546f, -0.097773f, -0.098997f, -0.100218f, -0.101435f, -0.102649f, -0.103859f, -0.105066f, 
-0.106269f, -0.107468f, -0.108664f, -0.109856f, -0.111045f, -0.112230f, -0.113411f, -0.114588f, -0.115762f, -0.116931f, -0.118097f, -0.119259f, 
-0.120417f, -0.121571f, -0.122721f, -0.123867f, -0.125009f, -0.126147f, -0.127281f, -0.128411f, -0.129537f, -0.130658f, -0.131775f, -0.132888f, 
-0.133997f, -0.135102f, -0.136202f, -0.137298f, -0.138389f, -0.139476f, -0.140559f, -0.141637f, -0.142710f, -0.143780f, -0.144844f, -0.145904f, 
-0.146960f, -0.148011f, -0.149057f, -0.150098f, -0.151135f, -0.152167f, -0.153195f, -0.154218f, -0.155235f, -0.156248f, -0.157257f, -0.158260f, 
-0.159259f, -0.160252f, -0.161241f, -0.162224f, -0.163203f, -0.164177f, -0.165145f, -0.166109f, -0.167068f, -0.168021f, -0.168969f, -0.169912f, 
-0.170850f, -0.171783f, -0.172711f, -0.173633f, -0.174550f, -0.175462f, -0.176369f, -0.177270f, -0.178166f, -0.179056f, -0.179941f, -0.180821f, 
-0.181695f, -0.182564f, -0.183427f, -0.184285f, -0.185137f, -0.185984f, -0.186825f, -0.187661f, -0.188491f, -0.189316f, -0.190134f, -0.190948f, 
-0.191755f, -0.192557f, -0.193353f, -0.194144f, -0.194928f, -0.195707f, -0.196480f, -0.197248f, -0.198009f, -0.198765f, -0.199515f, -0.200259f, 
-0.200997f, -0.201730f, -0.202456f, -0.203176f, -0.203891f, -0.204599f, -0.205302f, -0.205998f, -0.206689f, -0.207373f, -0.208052f, -0.208724f, 
-0.209391f, -0.210051f, -0.210705f, -0.211353f, -0.211995f, -0.212631f, -0.213261f, -0.213884f, -0.214502f, -0.215113f, -0.215718f, -0.216316f, 
-0.216909f, -0.217495f, -0.218075f, -0.218649f, -0.219216f, -0.219778f, -0.220332f, -0.220881f, -0.221423f, -0.221959f, -0.222489f, -0.223012f, 
-0.223529f, -0.224039f, -0.224543f, -0.225041f, -0.225532f, -0.226017f, -0.226495f, -0.226967f, -0.227433f, -0.227892f, -0.228344f, -0.228791f, 
-0.229230f, -0.229663f, -0.230090f, -0.230510f, -0.230924f, -0.231331f, -0.231732f, -0.232126f, -0.232513f, -0.232894f, -0.233269f, -0.233637f, 
-0.233998f, -0.234353f, -0.234701f, -0.235042f, -0.235377f, -0.235706f, -0.236028f, -0.236343f, -0.236651f, -0.236953f, -0.237249f, -0.237537f, 
-0.237820f, -0.238095f, -0.238364f, -0.238626f, -0.238882f, -0.239131f, -0.239373f, -0.239609f, -0.239838f, -0.240060f, -0.240276f, -0.240485f, 
-0.240687f, -0.240883f, -0.241072f, -0.241254f, -0.241430f, -0.241599f, -0.241761f, -0.241917f, -0.242066f, -0.242208f, -0.242344f, -0.242473f, 
-0.242596f, -0.242711f, -0.242820f, -0.242923f, -0.243018f, -0.243107f, -0.243190f, -0.243266f, -0.243335f, -0.243397f, -0.243453f, -0.243502f, 
-0.243544f, -0.243580f, -0.243609f, -0.243632f, -0.243648f, -0.243657f, -0.243660f, -0.243656f, -0.243645f, -0.243628f, -0.243604f, -0.243574f, 
-0.243537f, -0.243493f, -0.243443f, -0.243386f, -0.243323f, -0.243253f, -0.243176f, -0.243093f, -0.243003f, -0.242907f, -0.242804f, -0.242695f, 
-0.242579f, -0.242457f, -0.242328f, -0.242193f, -0.242051f, -0.241902f, -0.241747f, -0.241586f, -0.241418f, -0.241244f, -0.241063f, -0.240876f, 
-0.240682f, -0.240482f, -0.240276f, -0.240063f, -0.239843f, -0.239618f, -0.239386f, -0.239147f, -0.238902f, -0.238651f, -0.238394f, -0.238130f, 
-0.237860f, -0.237583f, -0.237300f, -0.237011f, -0.236716f, -0.236414f, -0.236107f, -0.235792f, -0.235472f, -0.235146f, -0.234813f, -0.234474f, 
-0.234129f, -0.233778f, -0.233420f, -0.233057f, -0.232687f, -0.232311f, -0.231929f, -0.231541f, -0.231147f, -0.230747f, -0.230341f, -0.229929f, 
-0.229511f, -0.229087f, -0.228657f, -0.228221f, -0.227779f, -0.227331f, -0.226877f, -0.226417f, -0.225951f, -0.225480f, -0.225002f, -0.224519f, 
-0.224030f, -0.223535f, -0.223034f, -0.222528f, -0.222016f, -0.221498f, -0.220974f, -0.220445f, -0.219910f, -0.219369f, -0.218823f, -0.218271f, 
-0.217713f, -0.217150f, -0.216581f, -0.216007f, -0.215427f, -0.214842f, -0.214251f, -0.213655f, -0.213053f, -0.212446f, -0.211833f, -0.211215f, 
-0.210592f, -0.209963f, -0.209329f, -0.208689f, -0.208045f, -0.207394f, -0.206739f, -0.206079f, -0.205413f, -0.204742f, -0.204066f, -0.203385f, 
-0.202698f, -0.202007f, -0.201310f, -0.200608f, -0.199902f, -0.199190f, -0.198473f, -0.197752f, -0.197025f, -0.196293f, -0.195557f, -0.194815f, 
};




#define WT_TOTAL_WAVETABLE_COUNT 2
#define WT_HZ 40.0f
#define WT_FRAMES_PER_CYCLE 1200
#define WT_SR 48000.0f
#define WT_HZ_RECIP (1.0f/WT_HZ)

typedef struct st_wavetable
{
    int length;
    float hz_recip;  
    float * wavetable;
}t_wavetable;

t_wavetable * g_wavetable_get();

t_wavetable * g_wavetable_get()
{
    t_wavetable * f_result;
    if(posix_memalign((void**)&f_result, 16, (sizeof(t_wavetable))) != 0){return 0;}
    if(posix_memalign((void**)&f_result->wavetable, 16, (sizeof(float) * WT_FRAMES_PER_CYCLE)) != 0){return 0;}
    f_result->hz_recip = WT_HZ_RECIP;
    f_result->length = WT_FRAMES_PER_CYCLE;
    return f_result;
}

typedef struct 
{
    t_wavetable ** tables;
    int f_count;
}t_wt_wavetables;

t_wt_wavetables * g_wt_wavetables_get(float);

t_wt_wavetables * g_wt_wavetables_get(float a_sr)
{
    int f_i = 0;
    t_wt_wavetables * f_result;
    if(posix_memalign((void**)&f_result, 16, (sizeof(t_wt_wavetables))) != 0){return 0;}
    f_result->f_count = WT_TOTAL_WAVETABLE_COUNT;
    if(posix_memalign((void**)&f_result->tables, 16, (sizeof(t_wavetable) * WT_TOTAL_WAVETABLE_COUNT)) != 0){return 0;}

    f_i = 0;
    while(f_i < WT_FRAMES_PER_CYCLE)
    {
        f_result->tables[f_i]->wavetable[f_i] = plain_saw_array[f_i]; 
        f_i++;
    }

    f_i = 0;
    while(f_i < WT_FRAMES_PER_CYCLE)
    {
        f_result->tables[1]->wavetable[f_i] = superbsaw_array[f_i]; 
        f_i++;
    }

    return f_result;
}



#ifdef	__cplusplus
}
#endif

#endif	/* WAVETABLES_H */

