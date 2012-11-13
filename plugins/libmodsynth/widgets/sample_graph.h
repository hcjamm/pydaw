/* 
 * File:   sample_graph.h
 * Author: Jeff Hubbard
 * 
 * Display a graph of an audio file in a QPixMap.
 *
 * Created on April 3, 2012, 8:40 PM
 */

#ifndef SAMPLE_GRAPH_H
#define	SAMPLE_GRAPH_H

#include <QPixmap>
#include <QPainter>
#include <QLabel>
#include <QWidget>
#include <QList>
#include <sndfile.h>
#include <math.h>
#include <QSize>

class LMS_sample_graph
{
public:
    LMS_sample_graph(int a_count, int a_graph_height, int a_graph_width, QWidget * a_parent)
    {
        lms_graph_height = a_graph_height;
        lms_graph_width = a_graph_width;
        lms_graph_count = a_count;        
        lms_parent = a_parent;
                
        m_sample_graph = new QLabel(a_parent);
        m_sample_graph->setObjectName(QString::fromUtf8("m_sample_graph"));
        m_sample_graph->setMinimumSize(QSize(a_graph_width, a_graph_height));
        m_sample_graph->setMaximumSize(QSize(a_graph_width, a_graph_height));
        m_sample_graph->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
        m_sample_graph->setStyleSheet(QString::fromUtf8("QLabel {background-color: white;};"));
        m_sample_graph->setAlignment(Qt::AlignLeading|Qt::AlignLeft|Qt::AlignTop);
        
        for(int f_i = 0; f_i < a_count; f_i++)
        {
            QPixmap * f_pixmap = new QPixmap();
            f_pixmap->fill();
            lms_graphs.append(*f_pixmap);
            lms_durations.append(0);
            m_sample_counts.append(0);
        }
    }
        
    QWidget * lms_parent;
    QList <QPixmap> lms_graphs;
    QList <float> lms_durations;   //TODO: implement this
    QList <int> m_sample_counts; //TODO: implement this
    QLabel * m_sample_graph;
    int lms_graph_count;
    int lms_graph_height;
    int lms_graph_width;
    
    void generatePreview(QString path, int a_index)  
    {
    SF_INFO info;
    SNDFILE *file;
    QPixmap pmap(lms_graph_width, lms_graph_height);
    pmap.fill();

    info.format = 0;
    file = sf_open(path.toLocal8Bit(), SFM_READ, &info);

    if (file && info.frames > 0) {

        float binSize = (float)info.frames / lms_graph_width;
        float peak[2] = { 0.0f, 0.0f }, mean[2] = { 0.0f, 0.0f };
        float *frame = (float *)malloc(info.channels * sizeof(float));
        int bin = 0;

        QPainter paint(&pmap);

        for (size_t i = 0; i < ((uint)(info.frames)); ++i) {

            sf_readf_float(file, frame, 1);

            if (fabs(frame[0]) > peak[0]) peak[0] = fabs(frame[0]);
            mean[0] += fabs(frame[0]);

            if (info.channels > 1) {
                if (fabs(frame[1]) > peak[1]) peak[1] = fabs(frame[1]);
                mean[1] += fabs(frame[1]);
            }

            if (i == size_t((bin + 1) * binSize)) {

                float silent = 1.0 / float(lms_graph_height);

                if (info.channels == 1) {
                    mean[1] = mean[0];
                    peak[1] = peak[0];
                }

                mean[0] /= binSize;
                mean[1] /= binSize;

                int m = lms_graph_height / 2;

                paint.setPen(QColor::fromRgb(30, 20, 20, 0));
                paint.drawLine(bin, m, bin, int(m - m * peak[0]));
                if (peak[0] > silent && peak[1] > silent) {
                    paint.drawLine(bin, m, bin, int(m + m * peak[1]));
                }

                paint.setPen(Qt::gray);
                paint.drawLine(bin, m, bin, int(m - m * mean[0]));
                if (mean[0] > silent && mean[1] > silent) {
                    paint.drawLine(bin, m, bin, int(m + m * mean[1]));
                }

                paint.setPen(QColor::fromRgb(30, 20, 20, 0));
                paint.drawPoint(bin, int(m - m * peak[0]));
                if (peak[0] > silent && peak[1] > silent) {
                    paint.drawPoint(bin, int(m + m * peak[1]));
                }

                mean[0] = mean[1] = 0.0f;
                peak[0] = peak[1] = 0.0f;

                ++bin;
            }
        }

        //int duration = int(100.0 * float(info.frames) / float(info.samplerate));
        //std::cout << "duration " << duration << std::endl;
        
        /*m_duration->setText(QString("%1.%2%3 sec")
                            .arg(duration / 100)
                            .arg((duration / 10) % 10)
                            .arg((duration % 10)));
        m_sampleRate->setText(QString("%1 Hz")
                            .arg(info.samplerate));
        m_channels->setText(info.channels > 1 ? (m_balance ? "stereo" : "stereo (to mix)") : "mono");
        if (m_balanceLabel) {
            m_balanceLabel->setText(info.channels == 1 ? "Pan:  " : "Balance:  ");
        }

        } else {
            m_duration->setText("0.00 sec");
            m_sampleRate->setText("");
            m_channels->setText("");
        }*/

        if (file) sf_close(file);

        lms_graphs[a_index] =  pmap;

        m_sample_graph->setPixmap(pmap);

        }
    }


    void indexChanged(int a_index)
    {
        m_sample_graph->setPixmap(lms_graphs[a_index]);
    }
    
    void clearPixmap(int a_index)
    {
        QPixmap * f_pixmap = new QPixmap();
        f_pixmap->fill();
                
        lms_graphs.removeAt(a_index);
        lms_graphs.insert(a_index, *f_pixmap);
        
        lms_durations.removeAt(a_index);
        lms_durations.insert(a_index, 0);
        
        m_sample_counts.removeAt(a_index);
        m_sample_counts.insert(a_index, 0);
        
        m_sample_graph->setPixmap(*f_pixmap);
    }
    
};

#endif	/* SAMPLE_GRAPH_H */

