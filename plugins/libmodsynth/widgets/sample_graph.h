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
#include <QMessageBox>
#include <QList>
#include <sndfile.h>
#include <math.h>

class LMS_sample_graph
{
public:
    LMS_sample_graph(int a_count, int a_graph_height, int a_graph_width, QWidget * a_parent, int a_max_sample_size)
    {
        lms_graph_height = a_graph_height;
        lms_graph_width = a_graph_width;
        lms_graph_count = a_count;
        lms_max_sample_size = a_max_sample_size;
        lms_parent = a_parent;
                
        m_sample_graph = new QLabel(a_parent);
        m_sample_graph->setObjectName(QString::fromUtf8("m_sample_graph"));
        m_sample_graph->setMinimumSize(QSize(0, 200));
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
    QList <int> lms_durations;   //TODO: get rid of this
    QList <int> m_sample_counts;
    QLabel * m_sample_graph;
    int lms_graph_count;
    int lms_graph_height;
    int lms_graph_width;
    int lms_max_sample_size;
       

    void generatePreview(QString path, int a_index)
    {
        if(a_index > lms_graph_count)
        {
            //std::cout << "a_index is greater than lms_graph_count, NOT generating a sample graph\n";
        }
        
        lms_graphs[a_index].fill();

        if (!path.isEmpty()) {

            SF_INFO info;
            SNDFILE *file;

            info.format = 0;
            file = sf_open(path.toLocal8Bit(), SFM_READ, &info);

            if (!file) {
                QMessageBox::warning
                    (lms_parent, "Couldn't load audio file",
                    QString("Couldn't load audio sample file '%1'").arg(path),
                    QMessageBox::Ok, 0);
                return;
            }

            if (info.frames > lms_max_sample_size) {
                QMessageBox::warning
                    (lms_parent, "Couldn't use audio file",
                    QString("Audio sample file '%1' is too large (%2 frames, maximum is %3)").arg(path).arg((int)info.frames).arg(lms_max_sample_size),
                    QMessageBox::Ok, 0);
                sf_close(file);
                return;
            } else {  /*Success*/
                //sf_close(file);
                info.format = 0;
                file = sf_open(path.toLocal8Bit(), SFM_READ, &info);

                if (file && info.frames > 0) {

                    float binSize = (float)info.frames / lms_graph_width;
                    float peak[2] = { 0.0f, 0.0f }, mean[2] = { 0.0f, 0.0f };
                    float *frame = (float *)malloc(info.channels * sizeof(float));
                    int bin = 0;

                    QPainter paint(&(lms_graphs[a_index]));

                    for (size_t i = 0; i < info.frames; ++i) {

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

                            paint.setPen(Qt::black);
                            paint.drawLine(bin, m, bin, int(m - m * peak[0]));
                            if (peak[0] > silent && peak[1] > silent) {
                                paint.drawLine(bin, m, bin, int(m + m * peak[1]));
                            }

                            paint.setPen(Qt::gray);
                            paint.drawLine(bin, m, bin, int(m - m * mean[0]));
                            if (mean[0] > silent && mean[1] > silent) {
                                paint.drawLine(bin, m, bin, int(m + m * mean[1]));
                            }

                            paint.setPen(Qt::black);
                            paint.drawPoint(bin, int(m - m * peak[0]));
                            if (peak[0] > silent && peak[1] > silent) {
                                paint.drawPoint(bin, int(m + m * peak[1]));
                            }

                            mean[0] = mean[1] = 0.0f;
                            peak[0] = peak[1] = 0.0f;

                            ++bin;
                        }
                    }

                    lms_durations[a_index] = int(100.0 * float(info.frames) / float(info.samplerate));

                    m_sample_counts[a_index] = info.frames;
                }

                m_sample_graph->setPixmap(lms_graphs[a_index]);

                if (file) sf_close(file);

                }
        }        
    }

    void indexChanged(int a_index)
    {
        m_sample_graph->setPixmap(lms_graphs[a_index]);
    }
    
    
};

#endif	/* SAMPLE_GRAPH_H */

