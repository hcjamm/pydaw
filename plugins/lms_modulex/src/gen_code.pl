#!/usr/bin/perl
$controls_count = 3;
$effects_count = 4;
$first_control_port = 4;
$created_by = "Created by gen_code.pl";

$ports = "";
$protected_slots = "";
$public_slots = "";
$protected_slots_imp = "";
$public_slots_imp = "";
$connects = "";
$i_get_control = "";
$v_control_changed = "";
$v_set_control = "";

$connect_port = "";


for($i = 0; $i < $effects_count; $i++)
{
	for($i2 = 0; $i2 < $controls_count; $i2++)
	{
		$public_slots .= "    void setFX" . $i . "knob$i2 (float val);\n";
		$protected_slots .= "    void fx" . $i . "knob" . $i2 . "Changed(int);\n";
		$connects .= "    connect(m_fx[" . $i . "]->lms_knob" . $i2 . "->lms_knob,  SIGNAL(valueChanged(int)), this, SLOT(fx" . $i . "knob" . $i2 . "Changed(int)));\n";
		$protected_slots_imp .= "void SynthGUI::fx" . $i . "knob".$i2."Changed(int value){ lms_value_changed(value, m_fx[".$i."]->lms_knob".$i2."); }\n";
		$public_slots_imp .= "void SynthGUI::setFX" . $i . "knob" . $i2 . "(float val){ lms_set_value(val, m_fx[". $i . "]->lms_knob" . $i2 ."); }\n";
	}
	$public_slots .= "    void setFX" . $i . "combobox (float val);\n";
	$protected_slots .= "    void fx" . $i . "comboboxChanged(int);\n";
	$connects .= "    connect(m_fx[" . $i . "]->lms_combobox->lms_combobox,  SIGNAL(currentIndexChanged(int)), this, SLOT(fx". $i . "comboboxChanged(int)));\n";
	$protected_slots_imp .= "void SynthGUI::fx".$i."comboboxChanged(int value){ lms_value_changed(value, m_fx[".$i."]->lms_combobox); m_fx[".$i."]->lms_combobox_changed(); }\n";
	$public_slots_imp .= "void SynthGUI::setFX" . $i . "combobox(float val){ lms_set_value(val, m_fx[" . $i . "]->lms_combobox); }\n";	

	$port = ($i * ($controls_count + 1)) + i2 + $first_control_port;
	for($i2 = 0; $i2 < $controls_count; $i2++)
	{	
		$i_get_control .= "    case ".$port.": return m_fx[".$i."]->lms_knob".$i2."->lms_get_value();\n";
		$v_control_changed .= "    case ".$port.": fx".$i."knob".$i2."Changed(a_value); break;\n";
		$v_set_control .= "   case ".$port.": setFX".$i."knob".$i2."(a_value); break;\n";
		$connect_port .= "        case ".$port.": plugin->fx_controls[".$i."][".$i2."] = data; break;\n";
		$port++;

	}
	
	$i_get_control .= "    case ". $port . ": return m_fx[".$i."]->lms_combobox->lms_get_value();\n";
	$v_control_changed .= "    case ". $port . ": fx".$i."comboboxChanged(a_value); break;\n";
	$v_set_control .= "    case ". $port . ": setFX".$i."combobox(a_value); break;\n";
	$connect_port .= "        case ".$port.": plugin->fx_controls[".$i."][".$i2."] = data; break;\n";
}


print "
//ports  $created_by

    //protected slot defs.  $created_by
$protected_slots

    //public slot defs.  $created_by
$public_slots

    //signal/slot connections $created_by
$connects

//protected slots $created_by
$protected_slots_imp

//public slots $created_by
$public_slots_imp

//$created_by
void SynthGUI::v_set_control(int a_port, float a_value)
{   
    switch (a_port) 
    {
	$v_set_control
    }
}

//$created_by
void SynthGUI::v_control_changed(int a_port, int a_value, bool a_suppress_host_update)
{    
    if(a_suppress_host_update)
        m_suppressHostUpdate = true;
       /*Add the controls you created to the control handler*/
    
    switch (a_port) {
	$v_control_changed
    }
    
    if(a_suppress_host_update)
        m_suppressHostUpdate = false;
}

//$created_by
int SynthGUI::i_get_control(int a_port)
{        
    switch (a_port) {
    $i_get_control
    default:
	return 0;
        break;
    }
}

//$created_by
$connect_port

";







